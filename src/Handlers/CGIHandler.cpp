#include "../../include/Handlers/CGIHandler.hpp"
#include "../../include/CgiProcess.hpp"
#include "../../include/Connection.hpp"
#include "../../include/Handlers/handler_utils.hpp"
#include "../../include/Settings.hpp"
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

CGIHandler::CGIHandler(const Location& conf,
    const VirtualHost& vh,
    const ErrorRenderer& error_renderer,
    const std::string& interpreter,
    Connection* conn)
    : conf_(conf)
    , vh_(vh)
    , error_renderer_(error_renderer)
    , interpreter_(interpreter)
    , conn_(conn)
{
}

CGIHandler::~CGIHandler() { }

static std::string toUpperHeader(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '-')
            out.push_back('_');
        else if (c >= 'a' && c <= 'z')
            out.push_back(static_cast<char>(c - 'a' + 'A'));
        else
            out.push_back(c);
    }
    return out;
}

static std::string dirNameOf(const std::string& path)
{
    size_t slash = path.rfind('/');
    if (slash == std::string::npos)
        return ".";
    if (slash == 0)
        return "/";
    return path.substr(0, slash);
}

// Build CGI environment vector. Each entry is "KEY=VALUE".
static void buildEnv(std::vector<std::string>& env,
    const HttpRequest& request,
    const VirtualHost& vh,
    const std::string& script_url_path,
    const std::string& script_fs_path,
    const std::string& query_string,
    size_t content_length)
{
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("SERVER_SOFTWARE=42webserv/0.1.0");
    env.push_back("SERVER_NAME=" + vh.getHostname());
    env.push_back("SERVER_PORT=" + vh.getPort());
    env.push_back("REQUEST_METHOD=" + request.getRequestLine().getMethod());
    env.push_back("SCRIPT_NAME=" + script_url_path);
    env.push_back("SCRIPT_FILENAME=" + script_fs_path);
    env.push_back("PATH_INFO=");
    env.push_back("QUERY_STRING=" + query_string);
    env.push_back("REDIRECT_STATUS=200"); // required by php-cgi
    env.push_back("REQUEST_URI=" + request.getRequestLine().getRequestTarget());

    std::stringstream cl;
    cl << content_length;
    env.push_back("CONTENT_LENGTH=" + cl.str());

    const std::string& ct = request.getFieldLines().get("Content-Type");
    if (!ct.empty())
        env.push_back("CONTENT_TYPE=" + ct);

    // HTTP_* for every request header.
    static const char* kPropagated[] = {
        "Host", "User-Agent", "Accept", "Accept-Language", "Accept-Encoding",
        "Cookie", "Referer", "Authorization", "Connection", NULL
    };
    for (size_t i = 0; kPropagated[i] != NULL; ++i) {
        const std::string& v = request.getFieldLines().get(kPropagated[i]);
        if (!v.empty())
            env.push_back("HTTP_" + toUpperHeader(kPropagated[i]) + "=" + v);
    }
}

static bool setNonBlockCloexec(int fd)
{
    int status_flags = fcntl(fd, F_GETFL);
    if (status_flags == -1)
        return false;
    if (fcntl(fd, F_SETFL, status_flags | O_NONBLOCK) == -1)
        return false;
    int descriptor_flags = fcntl(fd, F_GETFD);
    if (descriptor_flags == -1)
        return false;
    if (fcntl(fd, F_SETFD, descriptor_flags | FD_CLOEXEC) == -1)
        return false;
    return true;
}

// Open a tempfile, write `body` to it, return the fd opened for reading.
// On failure, returns -1 and sets out_path to empty.
static int makeBodyFd(const std::string& body, std::string& out_path)
{
    if (body.empty()) {
        out_path.clear();
        int fd = open("/dev/null", O_RDONLY);
        if (fd == -1)
            return -1;
        return fd;
    }

    std::string tmpl = std::string(Settings::CGI_TMP_DIR) + "/webserv_cgi_XXXXXX";
    std::vector<char> tplbuf(tmpl.begin(), tmpl.end());
    tplbuf.push_back('\0');
    int wfd = mkstemp(&tplbuf[0]);
    if (wfd == -1) {
        out_path.clear();
        return -1;
    }
    out_path.assign(&tplbuf[0]);

    size_t off = 0;
    while (off < body.size()) {
        ssize_t n = write(wfd, body.data() + off, body.size() - off);
        if (n <= 0) {
            close(wfd);
            unlink(out_path.c_str());
            out_path.clear();
            return -1;
        }
        off += static_cast<size_t>(n);
    }
    close(wfd);

    int rfd = open(out_path.c_str(), O_RDONLY);
    if (rfd == -1) {
        unlink(out_path.c_str());
        out_path.clear();
        return -1;
    }
    return rfd;
}

// Read file contents into a string (used for chunked-uploaded bodies that
// the parser already streamed to disk).
static bool readFileFully(const std::string& path, std::string& out)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1)
        return false;
    char buf[8192];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0)
        out.append(buf, static_cast<size_t>(n));
    close(fd);
    return n >= 0;
}

HttpResponse* CGIHandler::handle(const HttpRequest& request)
{
    if (conn_ == NULL) {
        std::cerr << "[CGI] handler invoked without connection\n";
        return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
    }
    if (!isMethodAllowed(request, conf_))
        return constructHttpErrorResponse(request, error_renderer_, Location::S_405);

    std::string script_fs_path;
    try {
        script_fs_path = constructPath(request, conf_);
    } catch (const std::exception& e) {
        std::cerr << "[CGI] path resolution failed: " << e.what() << '\n';
        return constructHttpErrorResponse(request, error_renderer_, Location::S_403);
    }
    std::string query_string = stripQueryURI(script_fs_path);
    if (!query_string.empty() && query_string[0] == '?')
        query_string.erase(0, 1);

    // Resolve to an absolute path so the post-chdir execve in the child does
    // not re-resolve a relative argv[1] against the script's own directory.
    char resolved[PATH_MAX];
    if (realpath(script_fs_path.c_str(), resolved) != NULL)
        script_fs_path = resolved;

    struct stat st;
    if (stat(script_fs_path.c_str(), &st) == -1)
        return constructHttpErrorResponse(request, error_renderer_, Location::S_404);
    if (!S_ISREG(st.st_mode))
        return constructHttpErrorResponse(request, error_renderer_, Location::S_403);

    std::string url_target = request.getRequestLine().getRequestTarget();
    std::string script_url_path = url_target;
    {
        size_t q = script_url_path.find('?');
        if (q != std::string::npos)
            script_url_path.erase(q);
    }

    // Resolve request body bytes. Either from the in-memory body or from the
    // file the chunked parser streamed to disk.
    std::string body_bytes;
    if (!request.getBody().getStoredPath().empty()) {
        if (!readFileFully(request.getBody().getStoredPath(), body_bytes)) {
            std::cerr << "[CGI] could not read uploaded body file\n";
            return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
        }
    } else {
        body_bytes = request.getBody().get();
    }

    std::string body_tmp_path;
    int body_fd = makeBodyFd(body_bytes, body_tmp_path);
    if (body_fd == -1) {
        std::cerr << "[CGI] could not stage request body to tempfile\n";
        return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        close(body_fd);
        if (!body_tmp_path.empty())
            unlink(body_tmp_path.c_str());
        std::cerr << "[CGI] pipe() failed\n";
        return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
    }
    if (!setNonBlockCloexec(pipefd[0])) {
        close(pipefd[0]);
        close(pipefd[1]);
        close(body_fd);
        if (!body_tmp_path.empty())
            unlink(body_tmp_path.c_str());
        std::cerr << "[CGI] failed to configure pipe fds\n";
        return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
    }

    std::vector<std::string> env_strs;
    buildEnv(env_strs, request, vh_, script_url_path, script_fs_path,
        query_string, body_bytes.size());

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        close(body_fd);
        if (!body_tmp_path.empty())
            unlink(body_tmp_path.c_str());
        std::cerr << "[CGI] fork() failed\n";
        return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
    }

    if (pid == 0) {
        // Child.
        dup2(body_fd, STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        close(body_fd);
        close(pipefd[0]);
        close(pipefd[1]);

        std::string script_dir = dirNameOf(script_fs_path);
        if (chdir(script_dir.c_str()) == -1)
            _exit(127);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(interpreter_.c_str()));
        argv.push_back(const_cast<char*>(script_fs_path.c_str()));
        argv.push_back(NULL);

        std::vector<char*> envp;
        envp.reserve(env_strs.size() + 1);
        for (size_t i = 0; i < env_strs.size(); ++i)
            envp.push_back(const_cast<char*>(env_strs[i].c_str()));
        envp.push_back(NULL);

        execve(interpreter_.c_str(), &argv[0], &envp[0]);
        _exit(127);
    }

    // Parent.
    close(pipefd[1]);
    close(body_fd);

    CgiProcess* cgi = new CgiProcess(pid, pipefd[0], body_tmp_path, request, conn_);
    conn_->setCgi(cgi);
    return NULL;
}

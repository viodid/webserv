#include "../include/CgiProcess.hpp"
#include "../include/Connection.hpp"
#include "../include/Handlers/handler_utils.hpp"
#include "../include/HttpResponse/StringBodySource.hpp"
#include "../include/Settings.hpp"
#include "../include/Utils.hpp"
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

CgiProcess::CgiProcess(pid_t pid,
    int stdout_fd,
    const std::string& body_tmp_path,
    const HttpRequest& /*request*/,
    Connection* owner)
    : pid_(pid)
    , stdout_fd_(stdout_fd)
    , body_tmp_path_(body_tmp_path)
    , started_at_ms_(static_cast<unsigned long>(currTimeMs()))
    , state_(Running)
    , owner_(owner)
{
}

CgiProcess::~CgiProcess()
{
    if (stdout_fd_ != -1) {
        close(stdout_fd_);
        stdout_fd_ = -1;
    }
    if (state_ == Running)
        killChild_();
    if (pid_ > 0)
        reapChild_();
    if (!body_tmp_path_.empty())
        unlink(body_tmp_path_.c_str());
}

int CgiProcess::getStdoutFd() const { return stdout_fd_; }
Connection* CgiProcess::getOwner() const { return owner_; }
CgiProcess::State CgiProcess::getState() const { return state_; }
bool CgiProcess::isTerminal() const { return state_ != Running; }

void CgiProcess::onReadable()
{
    if (state_ != Running)
        return;

    char buf[4096];
    ssize_t n = read(stdout_fd_, buf, sizeof(buf));
    if (n > 0) {
        if (output_.size() + static_cast<size_t>(n) > Settings::CGI_MAX_OUTPUT_BYTES) {
            std::cerr << "[CGI] output cap exceeded; killing pid " << pid_ << '\n';
            killChild_();
            state_ = Failed;
            return;
        }
        output_.insert(output_.end(), buf, buf + n);
        return;
    }
    if (n == 0) {
        // EOF: child closed stdout. Reap to learn exit status.
        reapChild_();
        state_ = Done;
        return;
    }
    // n == -1: per spec we cannot inspect errno after read. Treat as failure
    // and let the loop deliver a 502.
    std::cerr << "[CGI] read error on pid " << pid_ << '\n';
    killChild_();
    state_ = Failed;
}

bool CgiProcess::checkTimeout(unsigned long now_ms)
{
    if (state_ != Running)
        return false;
    if (now_ms - started_at_ms_ < Settings::CGI_TIMEOUT_MS)
        return false;
    std::cerr << "[CGI] timeout; killing pid " << pid_ << '\n';
    killChild_();
    state_ = TimedOut;
    return true;
}

void CgiProcess::killChild_()
{
    if (pid_ > 0)
        kill(pid_, SIGKILL);
}

void CgiProcess::reapChild_()
{
    if (pid_ <= 0)
        return;
    int status = 0;
    pid_t r;
    do {
        r = waitpid(pid_, &status, 0);
    } while (r == -1 && errno == EINTR);
    pid_ = -1;
}

// Trim leading and trailing horizontal whitespace (also CR) from a string.
static std::string trimWs(const std::string& s)
{
    size_t a = 0;
    while (a < s.size() && (s[a] == ' ' || s[a] == '\t'))
        ++a;
    size_t b = s.size();
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r'))
        --b;
    return s.substr(a, b - a);
}

// Locate the end of the CGI header block. Accepts \n\n or \r\n\r\n.
// Returns std::string::npos if no terminator is found, otherwise returns
// the index of the first byte AFTER the terminator (i.e. start of body).
static size_t findHeaderEnd(const std::vector<char>& v, size_t& header_len)
{
    for (size_t i = 0; i + 1 < v.size(); ++i) {
        if (v[i] == '\n' && v[i + 1] == '\n') {
            header_len = i;
            return i + 2;
        }
        if (i + 3 < v.size()
            && v[i] == '\r' && v[i + 1] == '\n'
            && v[i + 2] == '\r' && v[i + 3] == '\n') {
            header_len = i;
            return i + 4;
        }
    }
    return std::string::npos;
}

static Location::StatusCodes parseStatusCode(const std::string& value)
{
    long code = std::strtol(value.c_str(), NULL, 10);
    switch (code) {
    case 200: return Location::S_200;
    case 201: return Location::S_201;
    case 301: return Location::S_301;
    case 302: return Location::S_302;
    case 400: return Location::S_400;
    case 403: return Location::S_403;
    case 404: return Location::S_404;
    case 405: return Location::S_405;
    case 408: return Location::S_408;
    case 413: return Location::S_413;
    case 414: return Location::S_414;
    case 415: return Location::S_415;
    case 500: return Location::S_500;
    case 501: return Location::S_501;
    case 502: return Location::S_502;
    case 503: return Location::S_503;
    case 504: return Location::S_504;
    default:  return Location::S_200;
    }
}

HttpResponse* CgiProcess::buildResponse(const HttpRequest& request,
    const ErrorRenderer& error_renderer)
{
    if (state_ == TimedOut)
        return constructHttpErrorResponse(request, error_renderer, Location::S_504);
    if (state_ == Failed)
        return constructHttpErrorResponse(request, error_renderer, Location::S_502);

    size_t header_len = 0;
    size_t body_start = findHeaderEnd(output_, header_len);
    if (body_start == std::string::npos) {
        std::cerr << "[CGI] missing header terminator; output bytes=" << output_.size() << '\n';
        return constructHttpErrorResponse(request, error_renderer, Location::S_502);
    }

    Location::StatusCodes status = Location::S_200;
    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Connection", "close");

    bool has_content_type = false;
    bool has_location = false;
    bool has_status = false;

    // Parse header lines from the buffered prefix.
    std::string headers(&output_[0], header_len);
    std::stringstream ss(headers);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            continue;
        size_t colon = line.find(':');
        if (colon == std::string::npos) {
            std::cerr << "[CGI] malformed header line, ignoring: " << line << '\n';
            continue;
        }
        std::string name = trimWs(line.substr(0, colon));
        std::string value = trimWs(line.substr(colon + 1));
        if (name == "Status") {
            status = parseStatusCode(value);
            has_status = true;
            continue;
        }
        if (name == "Content-Type" || name == "content-type")
            has_content_type = true;
        if (name == "Location" || name == "location")
            has_location = true;
        field_lines.set(name, value);
    }

    if (has_location && !has_status)
        status = Location::S_302;
    if (!has_content_type)
        field_lines.set("Content-Type", "text/html");

    std::string body;
    if (body_start < output_.size())
        body.assign(&output_[body_start], output_.size() - body_start);

    std::stringstream cl;
    cl << body.size();
    field_lines.set("Content-Length", cl.str());

    HttpResponse* response = new HttpResponse;
    response->setStatusLine(StatusLine(request.getRequestLine().getHttpVersion(), status));
    response->setFieldLines(field_lines);
    response->setBodySource(new StringBodySource(body));
    return response;
}

#pragma once
#include "Config.hpp"
#include "HttpRequest/HttpRequest.hpp"
#include "HttpResponse/HttpResponse.hpp"
#include "Handlers/ErrorRenderer.hpp"
#include <string>
#include <sys/types.h>
#include <vector>

class Connection;

/**
 * @class CgiProcess
 * @brief Owns a single forked CGI invocation and its stdout pipe.
 *
 * Lifecycle: Running -> (Done | Failed | TimedOut). The owning Connection
 * polls stdout via onReadable() until EOF, then calls buildResponse() to
 * synthesize an HttpResponse from the buffered output. The destructor reaps
 * any surviving child, closes fds, and removes the body tempfile.
 *
 * The instance does NOT manage its own poll registration; callers register
 * the stdout fd with EventManager and remove it on completion.
 */
class CgiProcess {
public:
    enum State {
        Running,
        Done,
        Failed,
        TimedOut
    };

    CgiProcess(pid_t pid,
        int stdout_fd,
        const std::string& body_tmp_path,
        const HttpRequest& request,
        Connection* owner);
    ~CgiProcess();

    int getStdoutFd() const;
    Connection* getOwner() const;
    State getState() const;
    bool isTerminal() const;

    /*
     * Reads available bytes from the stdout pipe into the output buffer.
     * Transitions to Done on EOF, Failed on read error or output cap exceeded.
     */
    void onReadable();

    /*
     * Returns true and transitions to TimedOut if the process has been
     * running for more than CGI_TIMEOUT_MS. Sends SIGKILL to the child.
     */
    bool checkTimeout(unsigned long now_ms);

    /*
     * Synthesize an HttpResponse from buffered output / state.
     * Caller takes ownership of the returned response.
     */
    HttpResponse* buildResponse(const HttpRequest& request,
        const ErrorRenderer& error_renderer);

private:
    pid_t pid_;
    int stdout_fd_;
    std::string body_tmp_path_;
    std::vector<char> output_;
    unsigned long started_at_ms_;
    State state_;
    Connection* owner_;

    void killChild_();
    void reapChild_();

    CgiProcess(const CgiProcess&);
    CgiProcess& operator=(const CgiProcess&);
};

unsigned long nowMs();

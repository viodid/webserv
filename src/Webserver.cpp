#include "../include/Webserver.hpp"
#include "../include/Handlers/handler_utils.hpp"

Webserver::Webserver(const std::vector<VirtualHost>& config)
    : config_(config)
{
}

Webserver::~Webserver()
{
    for (size_t i = 0; i < connections_.size(); i++)
        delete connections_[i];
}

void Webserver::init()
{
    for (std::vector<VirtualHost>::const_iterator it = config_.begin();
        it != config_.end(); it++) {
        Socket* socket_ptr = new Socket(it->getHostname(), it->getPort());
        Connection* connection_ptr = new Connection(Connection::LISTENER, socket_ptr, *it);
        connections_.push_back(connection_ptr);
    }
}

void Webserver::run()
{
    EventManager notifier(connections_);
    for (;;) {
        notifier.manage();
        const std::vector<pollfd>& pollfds = notifier.getPollFds();
        for (size_t i = 0; i < connections_.size(); i++) {
            Connection* c = notifier.getConnectionFor(pollfds[i].fd);
            if (pollfds[i].revents & POLLIN) {
#if DEBUG
                std::cout << "POLLIN - connection: " << i << " - fd: " << c->getFd() << '\n';
#endif
                handleRead_(notifier, *c);
            }
            if (pollfds[i].revents & POLLOUT) {
#if DEBUG
                std::cout << "POLLOUT - connection: " << i << " - fd: " << c->getFd() << '\n';
#endif
                handleWrite_(notifier, *c);
            }
            if (pollfds[i].revents & (POLLERR | POLLHUP)) {
#if DEBUG
                std::cout << "POLLERR - connection: " << i << " - fd: " << c->getFd() << '\n';
#endif
                handleClosedConn_(notifier, *c);
            }
        }
    }
}

void Webserver::handleNewClient_(EventManager& notifier, const Connection& c)
{
    int cfd = c.acceptNewConnection();
    notifier.addFd(cfd);
    Socket* socket_ptr = new Socket(cfd);
    Connection* connection_ptr = new Connection(Connection::CLIENT, socket_ptr, c.getConfig());
    connections_.push_back(connection_ptr);
}

void Webserver::handleClosedConn_(EventManager& manager, const Connection& c)
{
    manager.removeFd(c.getFd());
#if DEBUG
    std::cout << "closed conn fd: " << c.getFd() << std::endl;
#endif
    for (size_t i = 0; i < connections_.size(); i++) {
        if (connections_[i] == &c) {
            delete connections_[i];
            connections_.erase(connections_.begin() + i);
            break;
        }
    }
}

void Webserver::respondWithError_(EventManager& notifier, Connection& c,
    Location::StatusCodes code)
{
    // The request may have failed before its HTTP version was parsed. Default
    // to 1.1 so the response line is well-formed.
    if (c.getRequest().getRequestLine().getHttpVersion().empty())
        c.getRequest().setRequestLine("GET", "/", "1.1");

    ErrorRenderer error_renderer(c.getConfig().getStatusCodes());
    HttpResponse* response = constructHttpErrorResponse(c.getRequest(),
        error_renderer, code);
    c.setResponse(response);
    notifier.enableWrite(c.getFd());
}

void Webserver::handleRead_(EventManager& notifier, Connection& c)
{
    if (c.getType() == Connection::LISTENER) {
        handleNewClient_(notifier, c);
        return;
    }

    // An error response is already queued; ignore further reads.
    if (c.hasResponse())
        return;

    try {
        c.getRequest().parseFromReader(c);

        if (c.getRequest().needsBodySink()) {
            IBodySink* sink = createBodySink(c.getRequest(), c.getConfig());
            c.getRequest().installBodySink(sink);
            c.getRequest().parseBuffered();
        }

        if (c.getRequest().isDone())
            notifier.enableWrite(c.getFd());
    } catch (const ExceptionBadFraming& e) {
        std::cerr << "400 bad framing: " << e.what() << '\n';
        respondWithError_(notifier, c, Location::S_400);
    } catch (const ExceptionPayloadTooLarge& e) {
        std::cerr << "413 payload too large: " << e.what() << '\n';
        respondWithError_(notifier, c, Location::S_413);
    } catch (const ExceptionBodyLength& e) {
        std::cerr << "400 bad content-length: " << e.what() << '\n';
        respondWithError_(notifier, c, Location::S_400);
    } catch (const ExceptionMalformedRequestLine& e) {
        std::cerr << "400 malformed request line: " << e.what() << '\n';
        respondWithError_(notifier, c, Location::S_400);
    } catch (const ExceptionMalformedFieldLine& e) {
        std::cerr << "400 malformed field line: " << e.what() << '\n';
        respondWithError_(notifier, c, Location::S_400);
    } catch (const ExceptionRequestTimeout& e) {
        std::cerr << "408 request timeout: " << e.what() << '\n';
        respondWithError_(notifier, c, Location::S_408);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return handleClosedConn_(notifier, c);
    }
}

void Webserver::handleWrite_(EventManager& notifier, Connection& c)
{
    if (!c.hasResponse() && !c.getRequest().isDone())
        return;

    try {
        if (!c.hasResponse()) {
            ErrorRenderer error_renderer(c.getConfig().getStatusCodes());
            IRequestHandler* handler = createHandler(c.getRequest(), c.getConfig(), error_renderer);
            HttpResponse* response = handler->handle(c.getRequest());
            delete handler;
            c.setResponse(response);
        }

        if (c.writeBufferSize() == 0)
            c.pullBodyChunk();

        c.sendBytes();

        if (c.isWriteDone())
            handleClosedConn_(notifier, c);
    } catch (const std::exception& e) {
        std::cerr << "handleWrite_: " << e.what() << '\n';
        handleClosedConn_(notifier, c);
    }
}

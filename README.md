*This project has been created as part of the 42 curriculum by dyunta, anmateo-.*

# webserv

## Description

`webserv` is an NGINX-inspired HTTP/1.1 web server written in C++98. It is single-threaded and event-driven, built on a Reactor pattern over `poll(2)` so a single process can multiplex many clients, listening sockets, CGI children, and disk I/O without blocking.

The goal of the project is to implement the network plumbing of a real web server from scratch — sockets, accept loop, request parsing, response generation, virtual hosts, CGI — using only a strict subset of the C++98 standard library and no external dependencies. Message framing follows RFC 9112; the configuration file syntax is a small subset of NGINX.

## Features

| Area     | Supported                                                       |
|----------|-----------------------------------------------------------------|
| Methods  | GET, HEAD, POST, PUT, DELETE                                    |
| Body     | Content-Length, chunked transfer-encoding (RFC 9112 §7.1)       |
| CGI      | non-blocking child I/O via the main poll loop, per extension    |
| Static   | file serving, autoindex, custom error pages                     |
| Upload   | `multipart/form-data`, configurable `upload_store`              |
| Routing  | virtual servers, location blocks, redirects (301/302)           |
| Limits   | per-server `client_max_body_size`                               |

## Instructions

### Build

- `make` — build the `webserv` binary (`g++ -std=c++98 -Wall -Wextra -Werror -Wpedantic`).
- `make re` — clean rebuild.
- `make fclean` — remove objects and binary.
- `make test` — build and run the GoogleTest unit suite. Requires `libgtest-dev`:
  ```sh
  sudo apt install libgtest-dev
  ```

### Run

`./webserv` takes **no arguments** and loads `resources/default.conf` relative to the current working directory. From the repo root:

```sh
make
./webserv
```

The default config exposes two virtual servers:

- `127.0.0.1:8080` — static files, file upload, CGI, and a redirect demo.
- `0.0.0.0:9090` — directory autoindex demo.

End-to-end tests live in `test/e2e/tests.py` and run against a live server:

```sh
./webserv &
python3 test/e2e/tests.py
```

## Configuration reference

Supported directives (NGINX-style subset):

- `server { ... }` — declares a virtual host.
- `listen <ip>:<port>;` — bind address (one per server).
- `client_max_body_size <bytes>;` — caps request body size.
- `error_page <code> <path>;` — override the body served for a status code.
- `location <path> { ... }` — route block. Inside a location:
  - `root <fs path>;` — filesystem root for this route.
  - `index <file>;` — default file when the request targets a directory.
  - `allowed_methods GET POST DELETE ...;` — whitelist of methods.
  - `autoindex on|off;` — enable/disable directory listings.
  - `upload_store <fs path>;` — destination directory for uploads.
  - `cgi_pass <ext> <interpreter path>;` — map a file extension to a CGI interpreter (e.g. `.py /usr/bin/python3`).
  - `return <code> <target>;` — send a redirect response.

## Project structure

```
include/   headers — Handlers/, HttpRequest/, HttpResponse/, Interfaces/, Config*, Webserver, EventManager
src/       implementations mirroring include/; main.cpp is the entry point
resources/ default.conf, html/ (error & autoindex templates), cgi-bin/ test scripts
test/      GoogleTest unit tests (test_*.cpp) + test/e2e/tests.py
Makefile   build / test / clean targets
```

## Resources

**Specs / RFCs**

- [RFC 9112 — HTTP/1.1 message syntax and framing](https://www.rfc-editor.org/rfc/rfc9112)
- [RFC 9110 — HTTP semantics](https://www.rfc-editor.org/rfc/rfc9110)
- [RFC 3875 — The Common Gateway Interface (CGI/1.1)](https://www.rfc-editor.org/rfc/rfc3875)

**Books and patterns**

- Beej's Guide to Network Programming — sockets, `poll(2)`, edge cases.
- *Pattern-Oriented Software Architecture, vol. 2* — the Reactor pattern, which `EventManager` implements.
- NGINX documentation — inspiration for the configuration file syntax.

**Man pages**

`poll(2)`, `socket(2)`, `accept(2)`, `recv(2)`, `send(2)`, `fcntl(2)`, `sigaction(2)`.

**AI usage**

AI assistance was limited to **code review and refactoring help**: reviewing pull requests, flagging bugs, and suggesting cleanups on the request-parsing, CGI, and chunked-upload modules. All implementation and architectural decisions were written by hand by the authors.

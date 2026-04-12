#pragma once
#include "../Config.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"
#include "ErrorRenderer.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <strstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

std::size_t readFile(char* buffer, size_t len, const std::string& path);
HttpResponse constructHttpErrorResponse(const HttpRequest& request,
    const ErrorRenderer& error_renderer,
    Location::ErrorPages error_no);
bool isMethodAllowed(const HttpRequest& request, const Location& location);

#pragma once
#include "../Config.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"
#include "ErrorRenderer.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

class ErrorRenderer;

HttpResponse* constructHttpErrorResponse(const HttpRequest& request,
    const ErrorRenderer& error_renderer,
    Location::StatusCodes error_no);
HttpResponse* constructHttpRedirectResponse(const HttpRequest& request,
    const std::string& location,
    Location::StatusCodes code);
bool isMethodAllowed(const HttpRequest& request, const Location& location);
std::string constructPath(const HttpRequest& request, const Location& location);
std::string renderDirListing(const std::string& path, const std::string& requested_path);
std::string normalizeURI(const std::string& uri);
std::string stripQueryURI(std::string& uri);
/*
 * Strips any query string from `target` and returns the file extension
 * (with leading dot) of the remaining path. Returns "" if no extension.
 */
std::string extensionOf(const std::string& target);

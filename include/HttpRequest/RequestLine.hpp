#pragma once
#include "../Exceptions.hpp"
#include "../Settings.hpp"
#include <cctype>
#include <set>
#include <string>
#include <vector>

/*
 * request-line = method SP request-target SP HTTP-version
 * GET /foo HTTP/1.1
 * https://www.rfc-editor.org/rfc/rfc9112#name-request-line
 */
class RequestLine {
public:
    RequestLine() { };

    const std::string& getMethod() const;
    const std::string& getRequestTarget() const;
    const std::string& getHttpVersion() const;

    void setMethod(const std::string&);
    void setRequestTarget(const std::string&);
    void setHttpVersion(const std::string&);

    int parse(const char* buffer, int length);

private:
    std::string method_;
    std::string request_target_;
    std::string http_version_;
};

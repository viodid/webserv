#pragma once
#include "../Config.hpp"

/*
 * https://www.rfc-editor.org/rfc/rfc9112#name-status-line
 */
class StatusLine {
public:
    StatusLine(const std::string& http_version, Location::ErrorPages status_code);

    std::string format() const;

private:
    const std::string http_version_;
    const Location::ErrorPages status_code_;
    const std::string reason_phrase_;
};

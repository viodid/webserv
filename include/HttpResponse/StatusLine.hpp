#pragma once
#include "../Config.hpp"
#include "../Settings.hpp"

/*
 * https://www.rfc-editor.org/rfc/rfc9112#name-status-line
 */
class StatusLine {
public:
    StatusLine();
    StatusLine(const std::string& http_version, Location::StatusCodes status_code);

    std::string format() const;

private:
    std::string http_version_;
    Location::StatusCodes status_code_;
    std::string reason_phrase_;
};

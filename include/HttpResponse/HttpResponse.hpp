#pragma once
#include "../HttpRequest/Body.hpp"
#include "../HttpRequest/FieldLines.hpp"
#include "StatusLine.hpp"

enum ResponseStatusCode {
    R_200 = 200,
    R_201 = 201,
    R_301 = 301
};

class HttpResponse {
public:
    HttpResponse(const StatusLine&, const FieldLines&, const Body&);

    std::string format() const;

private:
    const StatusLine status_line_;
    const FieldLines field_lines_;
    const Body body_;
};

std::string generateResponseStatusMsg(ResponseStatusCode status_code);

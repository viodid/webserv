#pragma once
#include "../HttpRequest/Body.hpp"
#include "../HttpRequest/FieldLines.hpp"
#include "StatusLine.hpp"
#include <sstream>
#include <stdexcept>

class HttpResponse {
public:
    HttpResponse(const StatusLine&, const FieldLines&, const Body&);

    std::string format() const;

private:
    const StatusLine status_line_;
    const FieldLines field_lines_;
    const Body body_;
};

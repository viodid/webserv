#pragma once
#include "../HttpRequest/Body.hpp"
#include "../HttpRequest/FieldLines.hpp"
#include "../Interfaces/IBodySource.hpp"
#include "StatusLine.hpp"
#include <sstream>
#include <stdexcept>

class HttpResponse {
public:
    ~HttpResponse();

    void setStatusLine(const StatusLine& sl);
    void setFieldLines(const FieldLines& fl);
    void setBodySource(IBodySource* bs);

    std::string getBytesHeader() const;

private:
    StatusLine status_line_;
    FieldLines field_lines_;
    IBodySource* body_source_;
};

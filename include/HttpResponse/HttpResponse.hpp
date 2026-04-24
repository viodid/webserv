#pragma once
#include "../HttpRequest/FieldLines.hpp"
#include "../Interfaces/IBodySource.hpp"
#include "StatusLine.hpp"
#include <string>

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void setStatusLine(const StatusLine& sl);
    void setFieldLines(const FieldLines& fl);
    void setBodySource(IBodySource* bs);

    std::string getBytesHeader() const;
    bool bodyHasMore() const;
    std::string nextBodyChunk();

private:
    StatusLine status_line_;
    FieldLines field_lines_;
    IBodySource* body_source_;

    HttpResponse(const HttpResponse&);
    HttpResponse& operator=(const HttpResponse&);
};

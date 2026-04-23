#include "../../include/HttpResponse/HttpResponse.hpp"

HttpResponse::~HttpResponse()
{
    delete body_source_;
}

void HttpResponse::setStatusLine(const StatusLine& sl)
{
    status_line_ = sl;
}
void HttpResponse::setFieldLines(const FieldLines& fl)
{
    field_lines_ = fl;
}
void HttpResponse::setBodySource(IBodySource* bs)
{
    body_source_ = bs;
}

#include "../../include/HttpResponse/HttpResponse.hpp"
#include "../../include/Settings.hpp"

HttpResponse::HttpResponse()
    : body_source_(NULL)
{
}

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
    delete body_source_;
    body_source_ = bs;
}

std::string HttpResponse::getBytesHeader() const
{
    return status_line_.format() + field_lines_.format() + Settings::LINE_DELIMETER;
}

bool HttpResponse::bodyHasMore() const
{
    if (body_source_ == NULL)
        return false;
    return !body_source_->isEmpty();
}

std::string HttpResponse::nextBodyChunk()
{
    if (body_source_ == NULL)
        return std::string();
    return body_source_->nextChunk();
}

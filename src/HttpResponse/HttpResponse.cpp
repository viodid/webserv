#include "../../include/HttpResponse/HttpResponse.hpp"

HttpResponse::~HttpResponse()
{
    delete body_source_;
}

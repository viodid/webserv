#include "../../include/HttpResponse/EmptyBodySource.hpp"

std::string EmptyBodySource::nextChunk()
{
    return std::string();
}

bool EmptyBodySource::isEmpty()
{
    return true;
}

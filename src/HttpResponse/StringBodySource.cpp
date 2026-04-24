#include "../../include/HttpResponse/StringBodySource.hpp"

StringBodySource::StringBodySource(const std::string& content)
    : content_(content)
    , consumed_(false)
{
}

std::string StringBodySource::nextChunk()
{
    if (consumed_)
        return std::string();
    consumed_ = true;
    return content_;
}

bool StringBodySource::isEmpty()
{
    return consumed_;
}

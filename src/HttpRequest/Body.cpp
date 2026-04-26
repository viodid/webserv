#include "../../include/HttpRequest/Body.hpp"
#include "../../include/Handlers/handler_utils.hpp"
#include <vector>

Body::Body()
    : content_length_(0)
    , chunked_inited_(false)
{
}

Body::Body(const std::string& body)
    : body_(body)
    , content_length_(0)
    , chunked_inited_(false)
{
}

const std::string& Body::get() const
{
    return body_;
}

void Body::set(const std::string& body)
{
    body_ = body;
}

std::string Body::format() const
{
    return body_;
}

int Body::parse(const char* buffer, size_t buf_len, const std::string& content_len)
{
    if (content_length_ == 0) {
        char* end = NULL;
        content_length_ = std::strtoul(content_len.c_str(), &end, 10);
        if (content_length_ == 0)
            throw ExceptionBodyLength("malformed 'Content-Length' header");
    }
    if (buf_len >= content_length_)
        body_ = std::string(buffer, std::min(static_cast<size_t>(buf_len), content_length_));

    return body_.size();
}

const std::string& Body::getStoredPath() const
{
    return stored_path_;
}

bool Body::isChunkedDone() const
{
    return decoder_.isDone();
}

int Body::parseChunked(const char* buffer, size_t buf_len,
    const std::string& upload_store,
    const std::string& target)
{
    if (!chunked_inited_) {
        if (upload_store.empty())
            throw ExceptionFileWrite("upload_store not configured for route");

        std::string clean = target;
        stripQueryURI(clean);
        size_t hash = clean.find('#');
        if (hash != std::string::npos)
            clean.erase(hash);
        clean = normalizeURI(clean);

        std::string basename = "upload.bin";
        size_t slash = clean.rfind('/');
        if (slash != std::string::npos && slash + 1 < clean.size())
            basename = clean.substr(slash + 1);
        else if (slash == std::string::npos && !clean.empty())
            basename = clean;

        stored_path_ = upload_store;
        if (!stored_path_.empty() && stored_path_[stored_path_.size() - 1] != '/')
            stored_path_ += "/";
        stored_path_ += basename;

        out_file_.open(stored_path_.c_str(), std::ios::binary | std::ios::trunc);
        if (!out_file_.is_open())
            throw ExceptionFileWrite("could not open upload file: " + stored_path_);
        chunked_inited_ = true;
    }

    std::vector<char> decoded;
    size_t consumed = decoder_.feed(buffer, buf_len, decoded);
    if (!decoded.empty()) {
        out_file_.write(&decoded[0], decoded.size());
        if (out_file_.fail())
            throw ExceptionFileWrite("write failed: " + stored_path_);
    }
    if (decoder_.isDone() && out_file_.is_open())
        out_file_.close();

    return static_cast<int>(consumed);
}

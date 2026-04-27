#include "../../include/HttpRequest/Body.hpp"
#include "../../include/Handlers/handler_utils.hpp"
#include <vector>

Body::Body()
    : content_length_(0)
    , bytes_received_(0)
    , chunked_inited_(false)
{
}

Body::Body(const std::string& body)
    : body_(body)
    , content_length_(0)
    , bytes_received_(body.size())
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

int Body::parse(const char* buffer, size_t buf_len, const std::string& content_len, size_t max_body_size)
{
    if (content_length_ == 0) {
        char* end = NULL;
        content_length_ = std::strtoul(content_len.c_str(), &end, 10);
        if (content_length_ == 0)
            throw ExceptionBodyLength("malformed 'Content-Length' header");
        if (max_body_size > 0 && content_length_ > max_body_size)
            throw ExceptionPayloadTooLarge("Content-Length exceeds client_max_body_size");
    }
    size_t remaining = content_length_ - bytes_received_;
    size_t take = std::min(buf_len, remaining);
    if (take > 0) {
        body_.append(buffer, take);
        bytes_received_ += take;
        if (max_body_size > 0 && bytes_received_ > max_body_size)
            throw ExceptionPayloadTooLarge("body exceeds client_max_body_size");
    }
    return static_cast<int>(take);
}

bool Body::isContentLengthDone() const
{
    return bytes_received_ >= content_length_;
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
    const std::string& target,
    size_t max_body_size)
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
        bytes_received_ += decoded.size();
        if (max_body_size > 0 && bytes_received_ > max_body_size)
            throw ExceptionPayloadTooLarge("chunked body exceeds client_max_body_size");
        out_file_.write(&decoded[0], decoded.size());
        if (out_file_.fail())
            throw ExceptionFileWrite("write failed: " + stored_path_);
    }
    if (decoder_.isDone() && out_file_.is_open())
        out_file_.close();

    return static_cast<int>(consumed);
}

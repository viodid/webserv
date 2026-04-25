#include "../../include/HttpRequest/ChunkedDecoder.hpp"
#include "../../include/Exceptions.hpp"
#include <algorithm>
#include <cstdlib>

ChunkedDecoder::ChunkedDecoder()
    : state_(SizeLine)
    , chunk_remaining_(0)
    , crlf_seen_(0)
{
}

bool ChunkedDecoder::isDone() const
{
    return state_ == Done_;
}

size_t ChunkedDecoder::feed(const char* buf, size_t len, std::vector<char>& out)
{
    size_t i = 0;
    while (i < len && state_ != Done_) {
        switch (state_) {
        case SizeLine: {
            partial_line_ += buf[i];
            ++i;
            const size_t s = partial_line_.size();
            if (s >= 2 && partial_line_[s - 2] == '\r' && partial_line_[s - 1] == '\n')
                parseSize_();
            break;
        }
        case Data: {
            size_t take = std::min(chunk_remaining_, len - i);
            if (take > 0)
                out.insert(out.end(), buf + i, buf + i + take);
            i += take;
            chunk_remaining_ -= take;
            if (chunk_remaining_ == 0)
                state_ = DataCrlf;
            break;
        }
        case DataCrlf: {
            char c = buf[i++];
            if (crlf_seen_ == 0) {
                if (c != '\r')
                    throw ExceptionBadFraming("expected CR after chunk data");
                crlf_seen_ = 1;
            } else {
                if (c != '\n')
                    throw ExceptionBadFraming("expected LF after chunk data");
                crlf_seen_ = 0;
                state_ = SizeLine;
                partial_line_.clear();
            }
            break;
        }
        case Trailer: {
            partial_line_ += buf[i];
            ++i;
            const size_t s = partial_line_.size();
            if (s >= 2 && partial_line_[s - 2] == '\r' && partial_line_[s - 1] == '\n') {
                if (s == 2)
                    state_ = Done_;
                else
                    partial_line_.clear();
            }
            break;
        }
        case Done_:
            break;
        }
    }
    return i;
}

void ChunkedDecoder::parseSize_()
{
    std::string line(partial_line_.begin(), partial_line_.end() - 2);
    std::string hex = line;
    size_t semi = line.find(';');
    if (semi != std::string::npos)
        hex = line.substr(0, semi);
    if (hex.empty())
        throw ExceptionBadFraming("empty chunk size");
    char* end = NULL;
    unsigned long n = std::strtoul(hex.c_str(), &end, 16);
    if (end != hex.c_str() + hex.size())
        throw ExceptionBadFraming("invalid hex chunk size");
    chunk_remaining_ = static_cast<size_t>(n);
    partial_line_.clear();
    if (n == 0)
        state_ = Trailer;
    else
        state_ = Data;
}

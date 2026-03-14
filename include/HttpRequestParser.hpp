#pragma once
#include "Config.hpp"
#include <string>
#include <vector>
#include <map>

// represents a single HTTP header field
class HttpFieldLine {
public:
	HttpFieldLine(const std::string&, const std::string&);
	const std::string& getFieldName() const;
	const std::string& getFieldValue() const;
private:
	std::string field_name_;
	std::string field_value_;
};

// represents the first line of an HTTP request
class HttpRequestLine {
public:
	HttpRequestLine(const std::string&, const std::string&, const std::string&);
	const std::string& getMethod() const;
	const std::string& getRequestTarget() const;
	const std::string& getHttpVersion() const;
private:
	std::string method_;
	std::string request_target_;
	std::string http_version_;
};

// result codes returned by HttpRequestParser
enum ParseState {
	PARSE_SUCCESS,
	PARSE_INCOMPLETE,       // Not enough data yet
	PARSE_ERROR,            // Generic error
	PARSE_BAD_REQUEST,      // 400 Bad Request
	PARSE_URI_TOO_LONG,     // 414 URI Too Long
	PARSE_ENTITY_TOO_LARGE, // 413 Payload Too Large
	PARSE_NOT_IMPLEMENTED   // 501 Not Implemented
};

/* ---------------------------------------------------------------------------
 * HttpRequest — data holder for a fully parsed HTTP 1.0/1.1 request
 *
 * An HTTP request is composed of:
 *   A request-line: GET /foo HTTP/1.1
 *   Field lines:    Host: example.com
 *   And an optional message body followed by a new line '\r\n'
 *   https://www.rfc-editor.org/rfc/rfc9112#name-field-syntax
 * --------------------------------------------------------------------------- */
struct HttpRequest {
	Location::AllowedMethods          method;        // Reuses the enum from Config.hpp
	std::string                       path;
	std::string                       query_string;
	std::string                       version;
	std::map<std::string, std::string> headers;      // Keys are lower-cased
	std::string                       body;

	ParseState  state;
	std::string error_msg;

	HttpRequest() : method(Location::GET), state(PARSE_INCOMPLETE) {}

	std::string getHeader(const std::string& key) const;
	bool        hasHeader(const std::string& key) const;
};

// stateless HTTP request parser
class HttpRequestParser {
public:
	static const size_t MAX_URI_LENGTH  = 8192;
	static const size_t MAX_HEADER_SIZE = 8192;

	// max_body_size comes from VirtualHost::getSocketSize()
	static HttpRequest parse(const std::string& raw_request, size_t max_body_size);
	static HttpRequest parseIncremental(const std::string& buffer, bool is_complete,
	                                    size_t max_body_size);

private:
	static bool parseRequestLine(const std::string& line, HttpRequest& req);
	static bool parseHeader(const std::string& line, HttpRequest& req);
	static bool parseHeaderSection_(const std::string& buffer, size_t first_line_end,
	                                size_t header_end, HttpRequest& req);
	static bool parseBody_(const std::string& body_data, bool is_complete,
	                       size_t max_body_size, HttpRequest& req);
	static bool parseContentLengthBody_(const std::string& body_data, bool is_complete,
	                                    size_t max_body_size, HttpRequest& req);
	static bool parseChunkedBody(const std::string& body_data, HttpRequest& req,
	                             size_t max_body_size);
	static bool isValidVersion(const std::string& version);
	static bool checkBodySize_(size_t body_size, size_t max_body_size, HttpRequest& req);
	static bool handleMissingContentLength_(bool is_complete, HttpRequest& req);
};

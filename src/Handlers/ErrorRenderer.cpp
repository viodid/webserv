#include "../../include/Handlers/ErrorRenderer.hpp"

ErrorRenderer::ErrorRenderer(const std::vector<std::pair<Location::ErrorPages, std::string> >& error_pages_)
{
    error_path_[Location::E_400] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::E_403] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::E_404] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::E_405] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::E_408] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::E_413] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::E_414] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::E_500] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::E_501] = Settings::ERROR_PAGE_PATH;
    for (std::vector<std::pair<Location::ErrorPages, std::string> >::const_iterator it = error_pages_.begin();
        it != error_pages_.end();
        it++)
        error_path_[it->first] = it->second;
}

std::string generateDefaultErrorPage(Location::ErrorPages status_code)
{
    std::string phrase;
    std::string description;

    switch (status_code) {
    case Location::E_400:
        phrase = "Bad Request";
        description = "The server could not understand the request due to invalid syntax.";
        break;
    case Location::E_403:
        phrase = "Forbidden";
        description = "You do not have permission to access this resource.";
        break;
    case Location::E_404:
        phrase = "Not Found";
        description = "The requested resource could not be found on this server.";
        break;
    case Location::E_405:
        phrase = "Method Not Allowed";
        description = "The specified HTTP method is not allowed for this resource.";
        break;
    case Location::E_408:
        phrase = "Request Timeout";
        description = "The server timed out waiting for the request.";
        break;
    case Location::E_413:
        phrase = "Content Too Large";
        description = "The request entity is larger than limits defined by server configuration.";
        break;
    case Location::E_414:
        phrase = "URI Too Long";
        description = "The URI provided was too long for the server to process.";
        break;
    case Location::E_500:
        phrase = "Internal Server Error";
        description = "The server encountered an unexpected condition.";
        break;
    case Location::E_501:
        phrase = "Not Implemented";
        description = "The server does not support the functionality required to fulfill the request.";
        break;
    case Location::_ERROR_COUNT:
        phrase = "Error code not found";
        description = "";
        break;
    }

    std::stringstream ss;
    ss << status_code;
    std::string codeStr = ss.str();

    const std::string placeholder_code = "{CODE}";
    const std::string placeholder_phrase = "{PHRASE}";
    const std::string placeholder_desc = "{DESCRIPTION}";
    const std::string path = Settings::ERROR_PAGE_PATH;

    char buf[Settings::PARSER_MAX_BUFFER_SIZE];
    ssize_t bytes = readFile(buf, Settings::PARSER_MAX_BUFFER_SIZE, path.c_str());
    if (bytes < Settings::PARSER_MAX_BUFFER_SIZE)
        buf[bytes + 1] = '\0';

    std::string templ(buf);
    // CODE
    templ.replace(templ.find(placeholder_code), placeholder_code.size(), codeStr);
    templ.replace(templ.find(placeholder_code), placeholder_code.size(), codeStr);
    // PHRASE
    templ.replace(templ.find(placeholder_phrase), placeholder_phrase.size(), phrase);
    templ.replace(templ.find(placeholder_phrase), placeholder_phrase.size(), phrase);
    // DESCRIPTION
    templ.replace(templ.find(placeholder_desc), placeholder_desc.size(), description);

    return templ;
}

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

std::string ErrorRenderer::render(Location::ErrorPages status_code)
{
    const std::string placeholder_code = "{CODE}";
    const std::string placeholder_phrase = "{PHRASE}";
    const std::string placeholder_desc = "{DESCRIPTION}";
    const std::string path = Settings::ERROR_PAGE_PATH;

    std::stringstream ss;
    ss << status_code;
    std::string codeStr = ss.str();
    std::pair<std::string, std::string> error_msg = generateDefaultErrorMsg(status_code);


    char buf[Settings::PARSER_MAX_BUFFER_SIZE];
    ssize_t bytes = readFile(buf, Settings::PARSER_MAX_BUFFER_SIZE, path.c_str());
    if (bytes < Settings::PARSER_MAX_BUFFER_SIZE)
        buf[bytes + 1] = '\0';

    std::string templ(buf);
    // CODE
    templ.replace(templ.find(placeholder_code), placeholder_code.size(), codeStr);
    templ.replace(templ.find(placeholder_code), placeholder_code.size(), codeStr);
    // PHRASE
    templ.replace(templ.find(placeholder_phrase), placeholder_phrase.size(), error_msg.first);
    templ.replace(templ.find(placeholder_phrase), placeholder_phrase.size(), error_msg.first);
    // DESCRIPTION
    templ.replace(templ.find(placeholder_desc), placeholder_desc.size(), error_msg.second);

    return templ;
}

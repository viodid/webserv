#include "../../include/Handlers/ErrorRenderer.hpp"

ErrorRenderer::ErrorRenderer(const std::vector<std::pair<Location::StatusCodes, std::string> >& error_pages_)
{
    error_path_[Location::S_400] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::S_403] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::S_404] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::S_405] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::S_408] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::S_413] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::S_414] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::S_500] = Settings::ERROR_PAGE_PATH;
    error_path_[Location::S_501] = Settings::ERROR_PAGE_PATH;
    for (std::vector<std::pair<Location::StatusCodes, std::string> >::const_iterator it = error_pages_.begin();
        it != error_pages_.end();
        it++)
        error_path_[it->first] = it->second;
}

std::string ErrorRenderer::render(Location::StatusCodes status_code) const
{
    const std::string placeholder_code = "{CODE}";
    const std::string placeholder_phrase = "{PHRASE}";
    const std::string placeholder_desc = "{DESCRIPTION}";
    File file(Settings::ERROR_PAGE_PATH);

    std::stringstream ss;
    ss << status_code;
    std::string codeStr = ss.str();
    std::pair<std::string, std::string> error_msg = generateDefaultStatusMsg(status_code);
    std::string templ = file.readFile();
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

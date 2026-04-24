#include "../../include/Handlers/handler_utils.hpp"
#include "../../include/HttpResponse/StringBodySource.hpp"
#include <cstring>
#include <dirent.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

HttpResponse* constructHttpErrorResponse(const HttpRequest& request,
    const ErrorRenderer& error_renderer,
    Location::StatusCodes error_no)
{
    std::string body_html = error_renderer.render(error_no);
    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Content-Type", "text/html; charset=utf-8");
    field_lines.set("Connection", "close");
    std::stringstream ss;
    ss << body_html.size();
    field_lines.set("Content-Length", ss.str());

    HttpResponse* response = new HttpResponse;
    response->setStatusLine(StatusLine(request.getRequestLine().getHttpVersion(), error_no));
    response->setFieldLines(field_lines);
    response->setBodySource(new StringBodySource(body_html));
    return response;
}

bool isMethodAllowed(const HttpRequest& request, const Location& location)
{
    for (std::vector<Location::AllowedMethods>::const_iterator it = location.getAllowedMethods().begin();
        it != location.getAllowedMethods().end();
        it++) {
        if (*it == location.methodFromString(request.getRequestLine().getMethod()))
            return true;
    }
    return false;
}

std::string constructPath(const HttpRequest& request, const Location& location)
{
    if (location.getRoot().empty() || location.getPath().empty())
        throw std::runtime_error("location.root or location.path cannot be empty");
    std::string request_target = normalizeURI(request.getRequestLine().getRequestTarget());
    std::string alias = location.getRoot();
    std::string route = location.getPath();
    if (route.size() > 1) // location route != "/"
        request_target.erase(0, route.size());
    alias.append(request_target);

    return alias;
}

/*
 * Mutates the input string if a query URL is found.
 * Returns the query URL
 */
std::string stripQueryURI(std::string& uri)
{
    size_t query_idx = uri.find('?');
    std::string query;
    if (query_idx != std::string::npos) {
        query = uri.substr(query_idx);
        uri.erase(query_idx);
    }
    return query;
}

std::string normalizeURI(const std::string& uri)
{
    std::vector<std::string> segments;
    std::stringstream ss(uri);
    std::string segment;
    while (std::getline(ss, segment, '/')) {
        if (segment == "." || segment.empty())
            continue;
        if (segment == "..") {
            if (segments.empty())
                throw ExceptionParentRootDirectory("parent root directory reached");
            segments.pop_back();
            continue;
        }
        segments.push_back(segment);
    }

    std::string normalize_uri;
    for (std::vector<std::string>::const_iterator it = segments.begin();
        it != segments.end();
        it++)
        normalize_uri.append("/" + *it);
    return normalize_uri;
}

std::string renderDirListing(const std::string& path, const std::string& requested_path)
{
    if (File(path).getType() != File::DIRECTORY)
        throw std::runtime_error("cannot dir list anything but a dir");

    // MAIN TEMPLATE
    const std::string placeholder_path = "{PATH}";
    const std::string placeholder_listing = "{LISTING}";
    // TABLE ROW
    const std::string placeholder_file_name = "{FILE_NAME}";
    const std::string placeholder_file_size = "{FILE_SIZE}";
    const std::string placeholder_file_url = "{FILE_URL}";
    const std::string placeholder_file_date = "{FILE_DATE}";
    const std::string placeholder_file_icon = "{FILE_ICON}";

    File main_templ(Settings::DIR_LISTINTG_PATH);

    DIR* directory = opendir(path.c_str());
    if (!directory)
        throw std::runtime_error(std::strerror(errno));

    std::set<std::string> table_rows_dir;
    std::set<std::string> table_rows_file;
    struct dirent* p_dir;
    while ((p_dir = readdir(directory))) {
        File table_row(Settings::TABLE_ROW_PATH);
        std::string name = std::string(p_dir->d_name);
        std::string url = requested_path + name;

        struct stat f_stat;
        std::string local_path = path + "/" + name;
        if (stat(local_path.c_str(), &f_stat) == -1)
            throw std::runtime_error(std::strerror(errno));

        // std::string date = std::to_string(std::localtime(&f_stat.st_mtim.tv_sec));
        std::stringstream ss;
        ss << f_stat.st_mtim.tv_sec;
        std::string date = ss.str();
        std::string size = "-";
        std::string icon = "📄";
        if (p_dir->d_type == DT_DIR) {
            icon = "📁";
            url += '/';
        } else {
            std::stringstream ss;
            ss << f_stat.st_size;
            size = ss.str();
        }
        std::string tr_template = table_row.readFile();
        // FILE NAME
        tr_template.replace(tr_template.find(placeholder_file_name), placeholder_file_name.size(), name);
        tr_template.replace(tr_template.find(placeholder_file_name), placeholder_file_name.size(), name);
        // FILE URL
        tr_template.replace(tr_template.find(placeholder_file_url), placeholder_file_url.size(), url);
        // FILE LAST UPDATED DATE
        tr_template.replace(tr_template.find(placeholder_file_date), placeholder_file_date.size(), date);
        // FILE SIZE
        tr_template.replace(tr_template.find(placeholder_file_size), placeholder_file_size.size(), size);
        // FILE ICON
        tr_template.replace(tr_template.find(placeholder_file_icon), placeholder_file_icon.size(), icon);

        if (p_dir->d_type == DT_DIR)
            table_rows_dir.insert(tr_template);
        else
            table_rows_file.insert(tr_template);
    }
    if (closedir(directory) == -1)
        throw std::runtime_error(std::strerror(errno));

    std::string main_template = main_templ.readFile();
    main_template.replace(main_template.find(placeholder_path), placeholder_path.size(), requested_path);
    main_template.replace(main_template.find(placeholder_path), placeholder_path.size(), requested_path);
    std::string table_rows_str;
    for (std::set<std::string>::const_iterator it = table_rows_dir.begin(); it != table_rows_dir.end(); it++)
        table_rows_str.append("<tr>" + *it + "</tr>");
    for (std::set<std::string>::const_iterator it = table_rows_file.begin(); it != table_rows_file.end(); it++)
        table_rows_str.append("<tr>" + *it + "</tr>");
    main_template.replace(main_template.find(placeholder_listing), placeholder_listing.size(), table_rows_str);

    return main_template;
}

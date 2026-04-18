#include "../../include/Handlers/handler_utils.hpp"

HttpResponse constructHttpErrorResponse(const HttpRequest& request,
    const ErrorRenderer& error_renderer,
    Location::StatusCodes error_no)
{
    std::string body_html = error_renderer.render(error_no);
    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Content-Type", "text/html; charset=utf-8");
    std::stringstream ss;
    ss << body_html.size();
    field_lines.set("Content-Length", ss.str());
    return HttpResponse(
        StatusLine(request.getRequestLine().getHttpVersion(), error_no),
        field_lines,
        Body(body_html));
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
    std::string alias = location.getRoot();
    std::string request_target = request.getRequestLine().getRequestTarget();
    std::string route = location.getPath();
    if (route.size() > 1) // location route != "/"
        request_target.erase(0, route.size());
    alias.append(request_target);

    return alias;
}

std::string renderDirListing(const std::string& path)
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

    std::vector<std::string> table_rows;
    struct dirent* p_dir;
    while ((p_dir = readdir(directory))) {
        struct stat f_stat;
        if (stat(path.c_str(), &f_stat) == -1)
            throw std::runtime_error(std::strerror(errno));

        File table_row(Settings::TABLE_ROW_PATH);
        std::string name = std::string(p_dir->d_name);
        std::string url = path + name;
        // std::string date = std::to_string(std::localtime(&f_stat.st_mtim.tv_sec));
        std::string date = std::to_string(f_stat.st_mtim.tv_sec);
        std::string size = "-";
        std::string icon = "📄";
        if (DT_DIR & p_dir->d_type) {
            icon = "📁";
        } else {
            size = std::to_string(f_stat.st_size);
        }
        std::string tr_template = table_row.readFile();
        // FILE NAME
        tr_template.replace(tr_template.find(placeholder_file_name), tr_template.size(), name);
        tr_template.replace(tr_template.find(placeholder_file_name), tr_template.size(), name);
        // FILE URL
        tr_template.replace(tr_template.find(placeholder_file_url), tr_template.size(), url);
        // FILE LAST UPDATED DATE
        tr_template.replace(tr_template.find(placeholder_file_date), tr_template.size(), date);
        // FILE SIZE
        tr_template.replace(tr_template.find(placeholder_file_size), tr_template.size(), size);
        // FILE ICON
        tr_template.replace(tr_template.find(placeholder_file_icon), tr_template.size(), icon);

        table_rows.push_back(tr_template);
    }
    std::string main_template = main_templ.readFile();
    main_template.replace(main_template.find(placeholder_path), main_template.size(), path);
    std::string table_rows_str;
    for (std::vector<std::string>::const_iterator it = table_rows.begin(); it != table_rows.end(); it++)
        table_rows_str.append(*it);
    main_template.replace(main_template.find(placeholder_listing), main_template.size(), table_rows_str);

    return main_template;
}

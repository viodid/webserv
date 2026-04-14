#include "../include/Config.hpp"
#include "../include/Exceptions.hpp"
#include <cstdlib>
#include <map>
#include <stdexcept>

Location::Location(const std::string path,
    const std::vector<AllowedMethods> methods,
    const std::string redirection_code,
    const std::string redirection_path,
    const std::string root,
    const std::string default_file,
    bool dir_listing,
    const std::string upload_store,
    const std::map<std::string, std::string>& cgi_map)
    : path_(path)
    , allowed_methods_(methods)
    , redirection_code_(redirection_code)
    , redirection_path_(redirection_path)
    , root_(root)
    , default_file_(default_file)
    , directory_listing_(dir_listing)
    , upload_store_(upload_store)
    , cgi_map_(cgi_map)
{
}

Location::AllowedMethods Location::methodFromString(const std::string& method)
{
    static const char* METHODS[] = {HTTP_METHODS(MAKE_STRING)};
    for (size_t i = 0; i < sizeof(METHODS) / sizeof(METHODS[0]); ++i) {
        if (method == METHODS[i])
            return static_cast<AllowedMethods>(i);
    }
    throw ExceptionParserError("Unsupported HTTP method: " + method);
}

Location::StatusCodes Location::statusCodeFromCode(const std::string& code)
{
    char* end;
    long value = std::strtol(code.c_str(), &end, 10);
    
    if (end == code.c_str() || *end != '\0') {
        throw ExceptionParserError("Unsupported error code: " + code);
    }
    
    static const int VALID_CODES[] = {STATUS_CODES(COLLECT_CODE)};
    static const size_t VALID_CODES_COUNT = sizeof(VALID_CODES) / sizeof(VALID_CODES[0]);
    
    for (size_t i = 0; i < VALID_CODES_COUNT; ++i) {
        if (VALID_CODES[i] == value) {
            return static_cast<Location::StatusCodes>(value);
        }
    }
    throw ExceptionParserError("Unsupported error code: " + code);
}

const std::string& Location::getPath() const
{
    return path_;
}
const std::vector<Location::AllowedMethods>& Location::getAllowedMethods() const
{
    return allowed_methods_;
}
const std::string& Location::getRedirectionCode() const
{
    return redirection_code_;
}
const std::string& Location::getRedirectionPath() const
{
    return redirection_path_;
}
const std::string& Location::getRoot() const
{
    return root_;
}
const std::string& Location::getDefaultFile() const
{
    return default_file_;
}
bool Location::isDirectoryListing() const
{
    return directory_listing_;
}
const std::string& Location::getUploadStore() const
{
    return upload_store_;
}
const std::map<std::string, std::string>& Location::getCgiMap() const
{
    return cgi_map_;
}

VirtualHost::VirtualHost(
    const std::string hostname,
    const std::string port,
    size_t socket_size,
    const std::vector<std::pair<Location::StatusCodes, std::string> > status_codes,
    const std::vector<Location> locations)
    : hostname_(hostname)
    , port_(port)
    , socket_size_(socket_size)
    , status_codes_(status_codes)
    , locations_(locations)
{
}

const std::string& VirtualHost::getHostname() const
{
    return hostname_;
}
const std::string& VirtualHost::getPort() const
{
    return port_;
}
size_t VirtualHost::getSocketSize() const
{
    return socket_size_;
}
const std::vector<std::pair<Location::StatusCodes, std::string> >& VirtualHost::getStatusCodes() const
{
    return status_codes_;
}
const std::vector<Location>& VirtualHost::getLocations() const
{
    return locations_;
}

Config::Config(const std::vector<VirtualHost> vh)
    : virtual_hosts_(vh)
{
}

const std::vector<VirtualHost>& Config::getVirtualHosts() const
{
    return virtual_hosts_;
}

// FIXME: test purposes
const Config create_mock_config()
{
    std::vector<VirtualHost> vh;
    // vh1
    std::vector<Location::AllowedMethods> methods1;
    methods1.push_back(Location::GET);
    std::vector<Location> l1;
    l1.push_back(Location("/", methods1, "", "", "/var/www/html", "/var/www/html/403.html", false));
    vh.push_back(VirtualHost("127.0.0.1", "5555", 100000, std::vector<std::pair<Location::StatusCodes, std::string> >(), l1));
    // vh2
    std::vector<Location::AllowedMethods> methods2;
    methods2.push_back(Location::GET);
    methods2.push_back(Location::POST);
    std::vector<Location> l2;
    l2.push_back(Location("/", methods2, "", "", "/var/www/html", "/var/www/html/403.html", false));
    vh.push_back(VirtualHost("localhost", "42069", 100000, std::vector<std::pair<Location::StatusCodes, std::string> >(), l2));

    return Config(vh);
}

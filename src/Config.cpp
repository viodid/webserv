#include "../include/Config.hpp"

Location::Location(const std::vector<AllowedMethods>& methods, std::string& redirection,
    std::string& root, std::string& default_file, bool dir_listing)
    : allowed_methods_(methods)
    , redirection_(redirection)
    , root_(root)
    , default_file_(default_file)
    , directory_listing_(dir_listing)
{
}

const std::vector<AllowedMethods>& Location::getAllowedMethods() const
{
    return allowed_methods_;
}
const std::string& Location::getRedirection() const
{
    return redirection_;
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

VirtualHost::VirtualHost(
    const std::string& hostname,
    const std::string& port,
    size_t socket_size,
    const std::vector<std::pair<ErrorPages, std::string>>& error_pages,
    const std::vector<Location>& locations)
    : hostname_(hostname)
    , port_(port)
    , socket_size_(socket_size)
    , error_pages_(error_pages)
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
const size_t VirtualHost::getSocketSize() const
{
    return socket_size_;
}
const std::vector<std::pair<ErrorPages, std::string>>& VirtualHost::getErrorPages() const
{
    return error_pages_;
}
const std::vector<Location>& VirtualHost::getLocations() const
{
    return locations_;
}

#include "../include/Config.hpp"

Location::Location(const std::vector<AllowedMethods> methods,
    const std::string redirection,
    const std::string root,
    const std::string default_file,
    bool dir_listing)
    : allowed_methods_(methods)
    , redirection_(redirection)
    , root_(root)
    , default_file_(default_file)
    , directory_listing_(dir_listing)
{
}

const std::vector<Location::AllowedMethods>& Location::getAllowedMethods() const
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
    const std::string hostname,
    const std::string port,
    size_t socket_size,
    const std::vector<std::pair<Location::ErrorPages, std::string> > error_pages,
    const std::vector<Location> locations)
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
size_t VirtualHost::getSocketSize() const
{
    return socket_size_;
}
const std::vector<std::pair<Location::ErrorPages, std::string> >& VirtualHost::getErrorPages() const
{
    return error_pages_;
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
    l1.push_back(Location(methods1, "", "/var/www/html", "/var/www/html/403.html", false));
    vh.push_back(VirtualHost("localhost", "55555", 100000, std::vector<std::pair<Location::ErrorPages, std::string> >(), l1));
    // vh2
    std::vector<Location::AllowedMethods> methods2;
    methods2.push_back(Location::GET);
    methods2.push_back(Location::POST);
    std::vector<Location> l2;
    l1.push_back(Location(methods2, "", "/var/www/html", "/var/www/html/403.html", false));
    vh.push_back(VirtualHost("localhost", "42069", 100000, std::vector<std::pair<Location::ErrorPages, std::string> >(), l2));

    return Config(vh);
}

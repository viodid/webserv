#pragma once
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

enum AllowedMethods {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE
};

enum ErrorPages {
    E_400,
    E_403,
    E_404,
    E_500
};

class Location {
public:
    Location(const std::vector<AllowedMethods>&, std::string&,
        std::string&, std::string&, bool);

    const std::vector<AllowedMethods>& getAllowedMethods() const;
    const std::string& getRedirection() const;
    const std::string& getRoot() const;
    const std::string& getDefaultFile() const;
    bool isDirectoryListing() const;

private:
    std::vector<AllowedMethods> allowed_methods_;
    std::string redirection_;
    std::string root_;
    std::string default_file_;
    bool directory_listing_;
};

class VirtualHost {
public:
    VirtualHost(const std::string&, const std::string&, size_t,
        const std::vector<std::pair<ErrorPages, std::string>>&,
        const std::vector<Location>&);

    const std::string& getHostname() const;
    const std::string& getPort() const;
    const size_t getSocketSize() const;
    const std::vector<std::pair<ErrorPages, std::string>>& getErrorPages() const;
    const std::vector<Location>& getLocations() const;

private:
    std::string hostname_;
    std::string port_;
    size_t socket_size_;
    std::vector<std::pair<ErrorPages, std::string>> error_pages_;
    std::vector<Location> locations_;
};

class Config {
public:
    Config(const std::vector<VirtualHost>&);

    const std::vector<VirtualHost>& getVirtualHosts() const;

private:
    std::vector<VirtualHost> virtual_hosts_;
};

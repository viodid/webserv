#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>


class Location {
public:
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
        E_405,
        E_408,
        E_413,
        E_414,
        E_500,
        E_501,
        E_502,
        E_503,
        E_504
    };
    Location(std::string, std::vector<AllowedMethods>,
        std::string, std::string, std::string, std::string, bool,
        std::string = "",
        const std::map<std::string, std::string>& = std::map<std::string, std::string>());

    static AllowedMethods methodFromString(const std::string& method);
    static ErrorPages       errorPageFromCode(const std::string& code);

    const std::string& getPath() const;
    const std::vector<AllowedMethods>& getAllowedMethods() const;
    const std::string& getRedirectionCode() const;
    const std::string& getRedirectionPath() const;
    const std::string& getRoot() const;
    const std::string& getDefaultFile() const;
    bool isDirectoryListing() const;
    const std::string& getUploadStore() const;
    const std::map<std::string, std::string>& getCgiMap() const;

private:
    std::string path_;
    std::vector<AllowedMethods> allowed_methods_;
    std::string redirection_code_;
    std::string redirection_path_;
    std::string root_;
    std::string default_file_;
    bool directory_listing_;
    std::string upload_store_;
    std::map<std::string, std::string> cgi_map_;
};

class VirtualHost {
public:
    VirtualHost(const std::string, const std::string, size_t,
        const std::vector<std::pair<Location::ErrorPages, std::string> >,
        const std::vector<Location>);

    const std::string& getHostname() const;
    const std::string& getPort() const;
    size_t getSocketSize() const;
    const std::vector<std::pair<Location::ErrorPages, std::string> >& getErrorPages() const;
    const std::vector<Location>& getLocations() const;

private:
    std::string hostname_;
    std::string port_;
    size_t socket_size_;
    std::vector<std::pair<Location::ErrorPages, std::string> > error_pages_;
    std::vector<Location> locations_;
};

class Config {
public:
    Config(const std::vector<VirtualHost>);

    const std::vector<VirtualHost>& getVirtualHosts() const;

private:
    std::vector<VirtualHost> virtual_hosts_;
};

const Config create_mock_config();

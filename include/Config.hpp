#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#define HTTP_METHODS(X) \
    X(GET) \
    X(HEAD) \
    X(POST) \
    X(PUT) \
    X(DELETE)

#define STATUS_CODES(X) \
    X(200) \
    X(201) \
    X(301) \
    X(400) \
    X(403) \
    X(404) \
    X(405) \
    X(408) \
    X(413) \
    X(414) \
    X(500) \
    X(501) \
    X(502) \
    X(503) \
    X(504)

#define MAKE_ENUM(m) m,
#define MAKE_STRING(m) #m,
#define MAKE_ENUM_CODE(code) S_##code = code,
#define COLLECT_CODE(code) code,

class Location {
public:
    enum AllowedMethods {
        HTTP_METHODS(MAKE_ENUM)
        _COUNT // trailing comma c++98
    };

    enum StatusCodes {
        STATUS_CODES(MAKE_ENUM_CODE)
        _STATUS_COUNT // trailing comma c++98
    };
    Location(std::string, std::vector<AllowedMethods>,
        std::string, std::string, std::string, std::string, bool,
        std::string = "",
        const std::map<std::string, std::string>& = std::map<std::string, std::string>());

    static AllowedMethods methodFromString(const std::string& method);
    static StatusCodes       errorPageFromCode(const std::string& code);

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
    const std::string path_;
    const std::vector<AllowedMethods> allowed_methods_;
    const std::string redirection_code_;
    const std::string redirection_path_;
    const std::string root_;
    const std::string default_file_;
    bool directory_listing_;
    const std::string upload_store_;
    const std::map<std::string, std::string> cgi_map_;
};

class VirtualHost {
public:
    VirtualHost(const std::string, const std::string, size_t,
        const std::vector<std::pair<Location::StatusCodes, std::string> >,
        const std::vector<Location>);

    const std::string& getHostname() const;
    const std::string& getPort() const;
    size_t getSocketSize() const;
    const std::vector<std::pair<Location::StatusCodes, std::string> >& getErrorPages() const;
    const std::vector<Location>& getLocations() const;

private:
    std::string hostname_;
    std::string port_;
    size_t socket_size_;
    std::vector<std::pair<Location::StatusCodes, std::string> > error_pages_;
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

#include "../include/ConfigParser.hpp"
#include <gtest/gtest.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

// ============================================================
// Helper: writes content to a temporary file and deletes it
// when it goes out of scope (RAII).
// ============================================================
struct TempConf {
    std::string path;
    TempConf(const std::string& content)
    {
        char tmpl[] = "/tmp/test_webserv_XXXXXX";
        int fd = mkstemp(tmpl); // Creates a unique temp file and returns its fd
        if (fd == -1)
            throw std::runtime_error("mkstemp failed");
        	const char* data = content.c_str();
        	const size_t size = content.size();
    		size_t totalWritten = 0;
        	while (totalWritten < size) {
        	ssize_t written = write(fd, data + totalWritten, size - totalWritten);
        	if (written == -1) {
        		close(fd);
        		unlink(tmpl);
        		throw std::runtime_error("write to tmpfile failed");
        	}
        	totalWritten += static_cast<size_t>(written);
        }
        close(fd);
        path = std::string(tmpl);
    }
    ~TempConf() { std::remove(path.c_str()); }
};

static Config parseConf(const std::string& content)
{
    TempConf tmp(content);
    return ConfigParser(tmp.path).parse();
}

// ============================================================
// listen — host:port (subject: "Define all interface:port pairs")
// ============================================================

TEST(ConfigParser, ListenParsesHostAndPort)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location / { root /var/www; }\n"
        "}\n");
    const VirtualHost& vh = cfg.getVirtualHosts()[0];
    EXPECT_EQ(vh.getHostname(), "127.0.0.1");
    EXPECT_EQ(vh.getPort(),     "8080");
}

TEST(ConfigParser, ListenPortOnlyDefaultsHostToAny)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 9090;\n"
        "    location / { root /var/www; }\n"
        "}\n");
    const VirtualHost& vh = cfg.getVirtualHosts()[0];
    EXPECT_EQ(vh.getHostname(), "0.0.0.0");
    EXPECT_EQ(vh.getPort(),     "9090");
}

TEST(ConfigParser, MissingListenThrows)
{
    EXPECT_THROW(
        parseConf("server {\n location / { root /x; }\n}\n"),
        std::runtime_error);
}

TEST(ConfigParser, InvalidPortThrows)
{
    EXPECT_THROW(
        parseConf("server { listen 127.0.0.1:99999; location / { root /x; } }\n"),
        std::runtime_error);
}

// ============================================================
// client_max_body_size (subject: "Set the maximum allowed size
// for client request bodies")
// ============================================================

TEST(ConfigParser, ClientMaxBodySizeParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    client_max_body_size 2097152;\n"
        "    location / { root /var/www; }\n"
        "}\n");
    EXPECT_EQ(cfg.getVirtualHosts()[0].getSocketSize(), static_cast<size_t>(2097152));
}

TEST(ConfigParser, ClientMaxBodySizeDefaultsTo1MiB)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location / { root /var/www; }\n"
        "}\n");
    EXPECT_EQ(cfg.getVirtualHosts()[0].getSocketSize(), static_cast<size_t>(1048576));
}

TEST(ConfigParser, ClientMaxBodySizeInvalidThrows)
{
    EXPECT_THROW(
        parseConf("server { listen 127.0.0.1:80; client_max_body_size abc; location / { root /x; } }\n"),
        std::runtime_error);
}

// ============================================================
// error_page (subject: "Set up default error pages")
// ============================================================

TEST(ConfigParser, ErrorPageParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    error_page 404 /errors/404.html;\n"
        "    error_page 500 /errors/500.html;\n"
        "    location / { root /var/www; }\n"
        "}\n");
    const std::vector<std::pair<Location::ErrorPages, std::string> >& ep =
        cfg.getVirtualHosts()[0].getErrorPages();
    ASSERT_EQ(ep.size(), static_cast<size_t>(2));
    EXPECT_EQ(ep[0].first,  Location::E_404);
    EXPECT_EQ(ep[0].second, "/errors/404.html");
    EXPECT_EQ(ep[1].first,  Location::E_500);
    EXPECT_EQ(ep[1].second, "/errors/500.html");
}

TEST(ConfigParser, UnknownErrorCodeThrows)
{
    EXPECT_THROW(
        parseConf("server { listen 127.0.0.1:80; error_page 999 /x.html; location / { root /x; } }\n"),
        std::runtime_error);
}

// ============================================================
// location — root (subject: "directory where the requested file
// should be located")
// ============================================================

TEST(ConfigParser, LocationRootParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location /static {\n"
        "        root /var/www/static;\n"
        "    }\n"
        "}\n");
    const Location& loc = cfg.getVirtualHosts()[0].getLocations()[0];
    EXPECT_EQ(loc.getPath(), "/static");
    EXPECT_EQ(loc.getRoot(), "/var/www/static");
}

// ============================================================
// location — index (subject: "default file to serve when the
// requested resource is a directory")
// ============================================================

TEST(ConfigParser, LocationIndexParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location / {\n"
        "        root /var/www;\n"
        "        index index.html;\n"
        "    }\n"
        "}\n");
    EXPECT_EQ(cfg.getVirtualHosts()[0].getLocations()[0].getDefaultFile(), "index.html");
}

// ============================================================
// location — autoindex (subject: "Enabling or disabling
// directory listing")
// ============================================================

TEST(ConfigParser, AutoindexOnParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location /files {\n"
        "        root /var/www/files;\n"
        "        autoindex on;\n"
        "    }\n"
        "}\n");
    EXPECT_TRUE(cfg.getVirtualHosts()[0].getLocations()[0].isDirectoryListing());
}

TEST(ConfigParser, AutoindexOffParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location / {\n"
        "        root /var/www;\n"
        "        autoindex off;\n"
        "    }\n"
        "}\n");
    EXPECT_FALSE(cfg.getVirtualHosts()[0].getLocations()[0].isDirectoryListing());
}

TEST(ConfigParser, AutoindexInvalidValueThrows)
{
    EXPECT_THROW(
        parseConf("server { listen 127.0.0.1:80; location / { autoindex maybe; } }\n"),
        std::runtime_error);
}

// ============================================================
// location — allowed_methods (subject: "List of accepted HTTP
// methods for the route")
// ============================================================

TEST(ConfigParser, AllowedMethodsGetPostDelete)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location /api {\n"
        "        root /var/www/api;\n"
        "        allowed_methods GET POST DELETE;\n"
        "    }\n"
        "}\n");
    const std::vector<Location::AllowedMethods>& m =
        cfg.getVirtualHosts()[0].getLocations()[0].getAllowedMethods();
    ASSERT_EQ(m.size(), static_cast<size_t>(3));
    EXPECT_EQ(m[0], Location::GET);
    EXPECT_EQ(m[1], Location::POST);
    EXPECT_EQ(m[2], Location::DELETE);
}

TEST(ConfigParser, AllowedMethodsDefaultsToGet)
{
    // subject says GET, POST, DELETE are required; if not specified, default = GET
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location / { root /var/www; }\n"
        "}\n");
    const std::vector<Location::AllowedMethods>& m =
        cfg.getVirtualHosts()[0].getLocations()[0].getAllowedMethods();
    ASSERT_EQ(m.size(), static_cast<size_t>(1));
    EXPECT_EQ(m[0], Location::GET);
}

TEST(ConfigParser, UnknownMethodThrows)
{
    EXPECT_THROW(
        parseConf("server { listen 127.0.0.1:80; location / { allowed_methods PATCH; } }\n"),
        std::runtime_error);
}

// ============================================================
// location — return (subject: "HTTP redirection")
// ============================================================

TEST(ConfigParser, ReturnWithoutCodeParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location /old {\n"
        "        return /new;\n"
        "    }\n"
        "}\n");
    const Location& loc = cfg.getVirtualHosts()[0].getLocations()[0];
    EXPECT_EQ(loc.getRedirectionPath(), "/new");
    EXPECT_TRUE(loc.getRedirectionCode().empty());
}

TEST(ConfigParser, ReturnWithCodeParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location /old {\n"
        "        return 301 /new;\n"
        "    }\n"
        "}\n");
    const Location& loc = cfg.getVirtualHosts()[0].getLocations()[0];
    EXPECT_EQ(loc.getRedirectionCode(), "301");
    EXPECT_EQ(loc.getRedirectionPath(), "/new");
}

// ============================================================
// location — upload_store (subject: "Uploading files from the
// clients to the server is authorized, and storage location
// is provided")
// ============================================================

TEST(ConfigParser, UploadStoreParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location /upload {\n"
        "        root /var/www/upload;\n"
        "        allowed_methods GET POST DELETE;\n"
        "        upload_store /var/www/upload;\n"
        "    }\n"
        "}\n");
    EXPECT_EQ(cfg.getVirtualHosts()[0].getLocations()[0].getUploadStore(),
              "/var/www/upload");
}

// ============================================================
// location — cgi_pass (subject: "Execution of CGI, based on
// file extension (for example .php)")
// ============================================================

TEST(ConfigParser, CgiPassParsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location /cgi-bin {\n"
        "        root /var/www/cgi-bin;\n"
        "        cgi_pass .php /usr/bin/php-cgi;\n"
        "        cgi_pass .py  /usr/bin/python3;\n"
        "    }\n"
        "}\n");
    const std::map<std::string, std::string>& cgi =
        cfg.getVirtualHosts()[0].getLocations()[0].getCgiMap();
    ASSERT_EQ(cgi.size(), static_cast<size_t>(2));
    EXPECT_EQ(cgi.at(".php"), "/usr/bin/php-cgi");
    EXPECT_EQ(cgi.at(".py"),  "/usr/bin/python3");
}

// ============================================================
// Multiple virtual hosts (subject: "listen to multiple ports
// to deliver different content")
// ============================================================

TEST(ConfigParser, MultipleServerBlocks)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location / { root /var/www; }\n"
        "}\n"
        "server {\n"
        "    listen 0.0.0.0:9090;\n"
        "    location / { root /var/alt; }\n"
        "}\n");
    ASSERT_EQ(cfg.getVirtualHosts().size(), static_cast<size_t>(2));
    EXPECT_EQ(cfg.getVirtualHosts()[0].getPort(), "8080");
    EXPECT_EQ(cfg.getVirtualHosts()[1].getPort(), "9090");
}

TEST(ConfigParser, MultipleLocationsInOneServer)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location / { root /var/www; }\n"
        "    location /api { root /var/api; allowed_methods GET POST; }\n"
        "    location /upload { root /var/up; upload_store /var/up; }\n"
        "}\n");
    EXPECT_EQ(cfg.getVirtualHosts()[0].getLocations().size(), static_cast<size_t>(3));
}

// ============================================================
// Comments and unknown directives (robustness)
// ============================================================

TEST(ConfigParser, HashCommentsIgnored)
{
    Config cfg = parseConf(
        "# This is a comment\n"
        "server {\n"
        "    listen 127.0.0.1:8080; # inline comment\n"
        "    location / { root /var/www; }\n"
        "}\n");
    EXPECT_EQ(cfg.getVirtualHosts()[0].getPort(), "8080");
}

TEST(ConfigParser, UnknownServerDirectiveSkipped)
{
    // server_name is not implemented but should not throw
    EXPECT_NO_THROW(
        parseConf(
            "server {\n"
            "    listen 127.0.0.1:8080;\n"
            "    server_name example.com;\n"
            "    location / { root /var/www; }\n"
            "}\n"));
}

TEST(ConfigParser, UnknownLocationDirectiveSkipped)
{
    EXPECT_NO_THROW(
        parseConf(
            "server {\n"
            "    listen 127.0.0.1:8080;\n"
            "    location / { root /var/www; expires 30d; }\n"
            "}\n"));
}

// ============================================================
// Error cases
// ============================================================

TEST(ConfigParser, EmptyFileThrows)
{
    EXPECT_THROW(parseConf(""), std::runtime_error);
}

TEST(ConfigParser, NoServerBlockThrows)
{
    EXPECT_THROW(parseConf("# just a comment\n"), std::runtime_error);
}

TEST(ConfigParser, NonExistentFileThrows)
{
    EXPECT_THROW(ConfigParser("/tmp/does_not_exist_12345.conf").parse(),
                 std::runtime_error);
}

TEST(ConfigParser, UnclosedServerBlockThrows)
{
    EXPECT_THROW(
        parseConf("server { listen 127.0.0.1:80; location / { root /x; }\n"),
        std::runtime_error);
}

TEST(ConfigParser, DefaultConfFileParsesSuccessfully)
{
    // Smoke-test the actual bundled config
    EXPECT_NO_THROW(ConfigParser("resources/default.conf").parse());
}

TEST(ConfigParser, RedirectWithCode302Parsed)
{
    Config cfg = parseConf(
        "server {\n"
        "    listen 127.0.0.1:8080;\n"
        "    location /redirect {\n"
        "        return 302 /;\n"
        "    }\n"
        "}\n");
    const Location& loc = cfg.getVirtualHosts()[0].getLocations()[0];
    EXPECT_EQ(loc.getRedirectionCode(), "302");
    EXPECT_EQ(loc.getRedirectionPath(), "/");
}

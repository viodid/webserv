#pragma once

namespace Settings {
const int PARSER_BUFFER_SIZE = 1024;
const int PARSER_MAX_BUFFER_SIZE = 1 << 14; // 16KiB
const int RESPONSE_BUFFER_SIZE = 1 << 15; // 32KiB
const unsigned long TIMEOUT_REQUEST_MS = 200;
const char LINE_DELIMETER[] = "\r\n";
const char END_MESSAGE[] = "\r\n\r\n";
const char DEFAULT_CONFIG_PATH[] = "/home/viodid/Documents/42/webserv/resources/default.conf";
const char ERROR_PAGE_PATH[] = "/home/viodid/Documents/42/webserv/resources/html/error_page_template.html";
const char DIR_LISTINTG_PATH[] = "/home/viodid/Documents/42/webserv/resources/html/dir_listing.html";
const char TABLE_ROW_PATH[] = "/home/viodid/Documents/42/webserv/resources/html/table_row_dir_listing.html";
}

#pragma once
#include <cstddef>

namespace Settings {
const int PARSER_BUFFER_SIZE = 1024;
const int PARSER_MAX_BUFFER_SIZE = 1 << 14; // 16KiB
const int RESPONSE_BUFFER_SIZE = 1 << 15; // 32KiB
const unsigned long TIMEOUT_REQUEST_MS = 2000;
const char LINE_DELIMETER[] = "\r\n";
const char END_MESSAGE[] = "\r\n\r\n";
const char DEFAULT_CONFIG_PATH[] = "resources/default.conf";
const unsigned long CGI_TIMEOUT_MS = 10000;
const size_t CGI_MAX_OUTPUT_BYTES = 8 * 1024 * 1024; // 8 MiB
const char CGI_TMP_DIR[] = "/tmp";
const char ERROR_PAGE_PATH[] = "resources/html/error_page_template.html";
const char DIR_LISTINTG_PATH[] = "resources/html/dir_listing.html";
const char TABLE_ROW_PATH[] = "resources/html/table_row_dir_listing.html";
}

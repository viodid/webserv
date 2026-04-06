#pragma once

namespace Settings {
const int PARSER_BUFFER_SIZE = 1024;
const int PARSER_MAX_BUFFER_SIZE = 1 << 14; // 16KiB
const unsigned long TIMEOUT_REQUEST_MS = 10;
const char LINE_DELIMETER[] = "\r\n";
const char END_MESSAGE[] = "\r\n\r\n";
}

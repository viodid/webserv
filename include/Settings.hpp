#pragma once

namespace Settings {
const int CONNECTION_BUFFER_SIZE = 1 << 10; // 10KiB
const int PARSER_BUFFER_SIZE = 8;
const int PARSER_MAX_BUFFER_SIZE = 1 << 10; // 10KiB
const char LINE_DELIMETER[] = "\r\n";
const char END_MESSAGE[] = "\r\n\r\n";
}

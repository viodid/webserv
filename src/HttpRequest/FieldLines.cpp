#include "../../include/HttpRequest/FieldLines.hpp"

static bool isToken(const std::string& str);

FieldLines::FieldLines(const std::map<std::string, std::string>& field_lines)
    : field_lines_(field_lines)
{
}

const std::string& FieldLines::get(const std::string& field_name) const
{
    static const std::string empty_field = "";
    std::map<std::string, std::string>::const_iterator it = field_lines_.find(toLower(field_name));
    if (it == field_lines_.end())
        return empty_field;
    return it->second;
}

void FieldLines::set(const std::string& field_name, const std::string& field_value)
{
    if (!isToken(field_name))
        throw ExceptionMalformedFieldLine("Field name invalid characters");
    std::map<std::string, std::string>::const_iterator it = field_lines_.find(toLower(field_name));
    if (it != field_lines_.end())
        field_lines_[toLower(field_name)] = it->second + "," + field_value;
    else
        field_lines_[toLower(field_name)] = field_value;
}

void FieldLines::forEach(void (*f)(const std::string&, const std::string&)) const
{
    for (std::map<std::string, std::string>::const_iterator it = field_lines_.begin();
        it != field_lines_.end();
        it++)
        f(it->first, it->second);
}

std::string FieldLines::format() const
{
    std::stringstream ss;
    for (std::map<std::string, std::string>::const_iterator it = field_lines_.begin();
        it != field_lines_.end();
        it++)
        ss << it->first << ": " << it->second << Settings::LINE_DELIMETER;
    return ss.str();
}

/*
 * field-line   = field-name ":" OWS field-value OWS
 *
 * Host: localhost:5555
 * User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:148.0)
 *
 * https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax
 */
int FieldLines::parse(const char* buffer, int length)
{
    std::string FIELD_DELIMETER = ":";
    const std::string str_stream(buffer, length);
    size_t delimeter_pos = str_stream.find(Settings::LINE_DELIMETER);

    if (delimeter_pos == 0)
        return 2;

    if (delimeter_pos == std::string::npos)
        return 0;

    std::vector<std::string> parts;
    // field-name
    size_t fl_delimeter = str_stream.find(FIELD_DELIMETER);
    if (fl_delimeter > delimeter_pos)
        throw ExceptionMalformedFieldLine("delimeter ':' not found");
    for (size_t i = 0; i < fl_delimeter; i++) {
        if (str_stream[i] == ' ')
            throw ExceptionMalformedFieldLine("malformed key header");
    }
    parts.push_back(str_stream.substr(0, fl_delimeter));

    // field-value
    std::string value = str_stream.substr(fl_delimeter + 1);
    size_t i;
    for (i = 0; i < value.size() && value[i] == ' '; i++) { }
    if (i == value.size())
        throw ExceptionMalformedFieldLine("header value not found");
    parts.push_back(value.substr(i, value.find(Settings::LINE_DELIMETER) - i));

    set(parts[0], parts[1]);

    return delimeter_pos + std::string(Settings::LINE_DELIMETER).size();
}

/*
 * https://datatracker.ietf.org/doc/html/rfc9110#name-tokens
 */
static bool isTokenCharacter(char c)
{
    if (std::isalnum(static_cast<unsigned char>(c)))
        return true;
    switch (c) {
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '*':
    case '+':
    case '-':
    case '.':
    case '^':
    case '_':
    case '`':
    case '|':
    case '~':
        return true;
    default:
        return false;
    }
}

static bool isToken(const std::string& str)
{
    if (str.empty())
        return false;
    for (size_t i = 0; i < str.size(); i++) {
        if (!isTokenCharacter(str[i]))
            return false;
    }
    return true;
}

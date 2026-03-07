#include "../include/ConfigParser.hpp"
#include <cstdlib>
#include <iostream>

// Constructor
ConfigParser::ConfigParser(const std::string& filepath)
    : filepath_(filepath)
    , pos_(0)
{
}

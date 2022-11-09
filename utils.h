#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <iostream>

namespace pcm
{

std::vector<std::string> split(const std::string & str, const char delim);
bool match(const std::string& subtoken, const std::string& sname, std::string& result);


}   // namespace pcm
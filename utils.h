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
class s_expect : public std::string
{
public:
    explicit s_expect(const char * s) : std::string(s) {}
    explicit s_expect(const std::string & s) : std::string(s) {}
    friend std::istream & operator >> (std::istream & istr, s_expect && s);
    friend std::istream & operator >> (std::istream && istr, s_expect && s);
private:

    void match(std::istream & istr) const;

};

FILE * tryOpen(const char * path, const char * mode);

std::string readSysFS(const char * path, bool silent);

bool writeSysFS(const char * path, const std::string & value, bool silent);

int readMaxFromSysFS(const char * path);

std::string safe_getenv(const char* env);

}   // namespace pcm
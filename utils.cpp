#include "utils.h"

namespace pcm
{

std::vector<std::string> split(const std::string & str, const char delim)
{
    std::string token;
    std::vector<std::string> result;
    std::istringstream strstr(str);
    while (std::getline(strstr, token, delim))
    {
        result.push_back(token);
    }
    return result;
}

bool match(const std::string& subtoken, const std::string& sname, std::string& result)
{

	std::regex rgx(subtoken);
	std::smatch matched;
	if (std::regex_search(sname.begin(), sname.end(), matched, rgx)){
		std::cout << "match: " << matched[1] << '\n';
		result= matched[1];
		return true;
	}
	return false;

}

void s_expect::match(std::istream & istr) const
{
    istr >> std::noskipws;
    const auto len = length();
    char * buffer = new char[len + 2];
    buffer[0] = 0;
    istr.get(buffer, len+1);
    if (*this != std::string(buffer))
    {
        istr.setstate(std::ios_base::failbit);
    }
    delete [] buffer;
}

inline std::istream & operator >> (std::istream & istr, s_expect && s)
{
    s.match(istr);
    return istr;
}

inline std::istream & operator >> (std::istream && istr, s_expect && s)
{
    s.match(istr);
    return istr;
}

FILE * tryOpen(const char * path, const char * mode)
{
    FILE * f = fopen(path, mode);
    if (!f)
    {
        f = fopen((std::string("/pcm") + path).c_str(), mode);
    }
    return f;
}

std::string readSysFS(const char * path, bool silent = false)
{
    FILE * f = tryOpen(path, "r");
    if (!f)
    {
        if (silent == false) std::cerr << "ERROR: Can not open " << path << " file.\n";
        return std::string();
    }
    char buffer[1024];
    if(NULL == fgets(buffer, 1024, f))
    {
        if (silent == false) std::cerr << "ERROR: Can not read from " << path << ".\n";
        fclose(f);
        return std::string();
    }
    fclose(f);
    return std::string(buffer);
}

bool writeSysFS(const char * path, const std::string & value, bool silent = false)
{
    FILE * f = tryOpen(path, "w");
    if (!f)
    {
        if (silent == false) std::cerr << "ERROR: Can not open " << path << " file.\n";
        return false;
    }
    if (fputs(value.c_str(), f) < 0)
    {
        if (silent == false) std::cerr << "ERROR: Can not write to " << path << ".\n";
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

}   // namespace pcm
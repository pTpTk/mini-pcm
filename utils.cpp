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


}   // namespace pcm
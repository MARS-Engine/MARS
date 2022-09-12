#include "string_helper.hpp"
#include <sstream>

std::vector<std::string> explode(std::string const & value, char delimeter) {
    std::vector<std::string> result;
    std::istringstream iss(value);

    for (std::string token; getline(iss, token, delimeter); )
        result.push_back(move(token));

    return result;
}
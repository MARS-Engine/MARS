#include "MARS/algo/string_helper.hpp"
#include <sstream>

std::vector<std::string> mars_string::explode(const std::string& _value, char _delimeter) {
    std::vector<std::string> result;
    std::istringstream iss(_value);

    for (std::string token; std::getline(iss, token, _delimeter); )
        result.push_back(std::move(token));

    return result;
}
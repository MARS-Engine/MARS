#include "string_helper.hpp"
#include <sstream>

vector<string> explode(string const & value, char delimeter) {
    vector<string> result;
    istringstream iss(value);

    for (string token; getline(iss, token, delimeter); )
        result.push_back(move(token));

    return result;
}
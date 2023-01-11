#ifndef MARS_STRING_
#define MARS_STRING_

#include <vector>
#include <string>

namespace mars_string {
    std::vector<std::string> explode(const std::string& _value, char _delimeter);
}

#endif
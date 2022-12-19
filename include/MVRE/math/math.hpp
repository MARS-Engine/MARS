#ifndef MVRE_MATH_
#define MVRE_MATH_

#include <string>
#include <array>
#include <algorithm>

namespace mvre_math {
    
    inline bool is_number(const std::string& s) {
        return !s.empty() && find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    }
}

#endif
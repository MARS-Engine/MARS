#ifndef __MVRE__MATH__
#define __MVRE__MATH__

#include <string>
#include <array>
#include <algorithm>

namespace mvre_math {
    
    inline bool is_number(const std::string& s) {
        return !s.empty() && find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    }
}

#endif
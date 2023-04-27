#ifndef MARS_MATH_
#define MARS_MATH_

#include <string>
#include <array>
#include <algorithm>
#include <cmath>

namespace mars_math {

    const float DEG2RAD = M_PI / 180.f;

    inline bool is_number(const std::string& s) {
        return !s.empty() && find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    }
}

#endif
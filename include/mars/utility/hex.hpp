#pragma once

#include <sstream>
#include <string_view>

namespace mars::utils {
    inline unsigned char hex_byte_to_char(const std::string_view& hex) {
        unsigned int value;
        std::stringstream ss;
        ss << std::hex << hex;
        ss >> value;
        return static_cast<unsigned char>(value);
    }
} // namespace mars::utils
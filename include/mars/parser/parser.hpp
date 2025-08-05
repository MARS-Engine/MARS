#pragma once

#include <ranges>
#include <string>
#include <string_view>

namespace mars {
    namespace parse {
        template <bool Negate = false>
        inline auto first_special(const std::string_view::iterator& begin, const std::string_view::iterator& end) {
            return std::ranges::find_if(begin, end, [](char c) -> bool {
                return (!std::isalnum(static_cast<unsigned char>(c)) && !std::isspace(static_cast<unsigned char>(c))) ^ Negate;
            });
        }

        template <bool Negate = false>
        inline auto first_space(const std::string_view::iterator& begin, const std::string_view::iterator& end) {
            return std::ranges::find_if(begin, end, [](char c) -> bool {
                // no idea why but doing a oneline makes this not work
                bool result = std::isspace(static_cast<unsigned char>(c));
                return result ^ !Negate;
            });
        }

        inline std::string extract_string(const std::string_view::iterator& _begin, const std::string_view::iterator& _end) {
            if (*_begin != '"' || (_begin + 1) == _end)
                return "";

            std::string_view::iterator current = std::ranges::find(_begin + 1, _end, '"');

            while (current != _end && *(current - 1) == '\\')
                current = std::ranges::find(_begin + 1, _end, '"');

            if (current == _end)
                return "";

            return std::string(_begin + 1, current);
        }
    } // namespace parse
} // namespace mars

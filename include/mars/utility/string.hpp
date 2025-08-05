#pragma once

#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace mars {

    template <char c>
    std::vector<std::string> split_string(const std::string_view& _input) {
        std::vector<std::string> result;

        for (auto part : std::views::split(_input, c))
            result.emplace_back(part.begin(), part.end());

        return result;
    }
}; // namespace mars
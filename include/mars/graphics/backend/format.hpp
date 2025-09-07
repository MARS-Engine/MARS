#pragma once

#include <climits>
#include <meta>
#include <optional>
#include <type_traits>

enum mars_format_type : unsigned char {
    MARS_FORMAT_TYPE_SFLOAT,
    MARS_FORMAT_TYPE_UINT,
};

namespace mars::graphics {
    struct format {
        mars_format_type type;
        unsigned char size;
        unsigned char number;
    };

    template <typename T>
    consteval format make_format() {
        format result = { .size = 0, .number = 0 };

        const auto get_type = [](std::meta::info& _info) -> mars_format_type {
            if (std::meta::is_same_type(_info, ^^float))
                return MARS_FORMAT_TYPE_SFLOAT;
            else if (std::meta::is_same_type(_info, ^^unsigned int))
                return MARS_FORMAT_TYPE_UINT;
            throw "unsupported type";
        };

        if constexpr (std::meta::is_class_type(^^T)) {
            std::optional<std::meta::info> value;

            std::meta::access_context ctx = std::meta::access_context::current();

            for (auto mem : std::meta::nonstatic_data_members_of(^^T, ctx)) {
                std::meta::info mem_type_info = std::meta::type_of(mem);

                if (value.has_value() && !std::meta::is_same_type(value.value(), mem_type_info))
                    throw "all non static data members must have the same type";

                result.type = get_type(mem_type_info);
                result.number++;
                result.size = std::meta::size_of(mem_type_info) * CHAR_WIDTH;
                value = mem_type_info;
            }
        } else {
            result.type = get_type(^^T);
            result.size = sizeof(T) * CHAR_WIDTH;
            result.number = 1;
        }

        return result;
    }
} // namespace mars::graphics
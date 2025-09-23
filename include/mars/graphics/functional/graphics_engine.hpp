#pragma once

#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {
    struct graphics_engine {
        graphics_backend_functions* allocator;

        template <typename T>
        static graphics_engine make() {
            graphics_engine result;
            result.allocator = T::get_functions();

            return result;
        }

        template <typename T>
        T& get_impl() {
            constexpr std::meta::access_context ctx = std::meta::access_context::current();
            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^graphics_backend_functions, ctx))) {
                if constexpr (std::meta::type_of(mem) == ^^T)
                    return allocator->[:mem:];
            }
            std::unreachable();
        }
    };
} // namespace mars
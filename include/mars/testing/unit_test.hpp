#pragma once

#include <mars/debug/logger.hpp>

#include <concepts>
#include <meta>

namespace mars::test_units {}

namespace mars::test {

inline mars::log_channel g_channel("test");

struct mars_test_context {};

template<auto = []{}>
void run_all() {
    template for (constexpr auto fn : std::define_static_array(std::meta::members_of(^^::mars::test_units))) {
        if constexpr (std::meta::is_function(fn)) {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(fn));
            if constexpr (params.size() == 0) {
                auto sublog = mars::logger::begin_sublog(g_channel, std::meta::identifier_of(fn));
                [:fn:]();
            }
        }
    }
}

template<typename Context, auto = []{}>
    requires std::derived_from<Context, mars_test_context>
void run_all(Context& _ctx) {
    template for (constexpr auto fn : std::define_static_array(std::meta::members_of(^^::mars::test_units))) {
        if constexpr (std::meta::is_function(fn)) {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(fn));
            if constexpr (params.size() == 0) {
                auto sublog = mars::logger::begin_sublog(g_channel, std::meta::identifier_of(fn));
                [:fn:]();
            } 
            else if constexpr (params.size() == 1) {
                using param_t = [:std::meta::type_of(params[0]):];
                if constexpr (std::is_same_v<std::remove_cvref_t<param_t>, Context>) {
                    auto sublog = mars::logger::begin_sublog(g_channel, std::meta::identifier_of(fn));
                    [:fn:](_ctx);
                }
            }
        }
    }
}

template<auto = []{}>
void list_all() {
    template for (constexpr auto fn : std::define_static_array(std::meta::members_of(^^::mars::test_units))) {
        if constexpr (std::meta::is_function(fn))
            mars::logger::log(g_channel, "  - {}", std::meta::identifier_of(fn));
    }
}

} // namespace mars::test

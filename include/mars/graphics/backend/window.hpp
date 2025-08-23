#pragma once

#include <mars/math/vector2.hpp>
#include <vector>

namespace mars {
    struct window;
    class graphics_engine;

    struct window_params {
        std::string title;
        mars::vector2<size_t> size;
    };

    struct window_impl {
        window (*window_create)(graphics_engine& _engine, const window_params& _params) = nullptr;
        void (*get_extensions)(const window& _window, std::vector<const char*>& _extensions) = nullptr;
    };
} // namespace mars
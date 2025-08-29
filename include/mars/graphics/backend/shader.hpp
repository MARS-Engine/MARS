#pragma once

#include <string_view>
#include <vector>

enum mars_shader_type {
    MARS_SHADER_TYPE_VERTEX,
    MARS_SHADER_TYPE_FRAGMENT
};

namespace mars {
    struct graphics_backend_functions;
    struct device;

    struct shader {
        graphics_backend_functions* engine;
        void* data = nullptr;
    };

    struct shader_module {
        mars_shader_type type;
        std::string_view path;
    };

    struct shader_impl {
        shader (*shader_create)(device& _device, const std::vector<shader_module>& _shaders) = nullptr;
        void (*shader_destroy)(shader& _shader, const device& _device) = nullptr;
    };
} // namespace mars
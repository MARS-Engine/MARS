#pragma once

#include <mars/math/vector3.hpp>
#include <string>

enum mars_graphics_message_severity {
    MARS_GRAPHICS_MESSAGE_SEVERITY_VERBOSE,
    MARS_GRAPHICS_MESSAGE_SEVERITY_INFO,
    MARS_GRAPHICS_MESSAGE_SEVERITY_WARN,
    MARS_GRAPHICS_MESSAGE_SEVERITY_ERROR,
};

namespace mars {
    struct graphics_engine;
    struct graphics_backend_functions;
    struct window;

    struct instance {
        graphics_backend_functions* engine;
        void* data = nullptr;
        bool debug_mode = false;
    };

    struct instance_create_params {
        std::string name;
        mars::vector3<size_t> app_version;
        bool debug_mode = false;
    };

    struct instance_impl {
        instance (*instance_create)(graphics_engine& _engine, const instance_create_params& _params, window& _window) = nullptr;
        void (*instance_destroy)(instance& _instance) = nullptr;
        void (*instance_listen_debug)(instance& _instance, void (*_callback)(instance& _instance, const std::string_view& _message, mars_graphics_message_severity _error_severity)) = nullptr;
    };
} // namespace mars
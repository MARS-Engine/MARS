#pragma once

#include <mars/graphics/backend/instance.hpp>
#include <mars/graphics/graphics_engine.hpp>
#include <mars/graphics/window.hpp>

namespace mars::graphics {
    inline instance instance_create(graphics_engine& _engine, const instance_create_params& _params, window& _window) {
        return _engine.get_impl<instance_impl>().instance_create(_engine, _params, _window);
    }

    inline void instance_destroy(instance& _instance) {
        _instance.engine->get_impl<instance_impl>().instance_destroy(_instance);
    }

    inline void instance_listen_debug(instance& _instance, void (*_callback)(instance& _instance, const std::string_view& _message, mars_graphics_message_severity _error_severity)) {
        _instance.engine->get_impl<instance_impl>().instance_listen_debug(_instance, _callback);
    }
} // namespace mars::graphics
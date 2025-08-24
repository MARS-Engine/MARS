
#include <mars/graphics/window.hpp>

#include <mars/graphics/graphics_engine.hpp>

namespace mars::graphics {
    window window_create(graphics_engine& _engine, const window_params& _params) {
        return _engine.get_impl<window_impl>().window_create(_engine, _params);
    }

    void window_create_surface(window& _window, instance& _instance) {
        return _window.engine->get_impl<window_impl>().window_create_surface(_window, _instance);
    }

    void window_get_extensions(const window& _window, std::vector<const char*>& _out) {
        return _window.engine->get_impl<window_impl>().window_get_extensions(_window, _out);
    }

    void window_destroy_surface(window& _window, instance& _instance) {
        return _window.engine->get_impl<window_impl>().window_destroy_surface(_window, _instance);
    }

    void window_destroy(window& _window) {
        return _window.engine->get_impl<window_impl>().window_destroy(_window);
    }
} // namespace mars::graphics
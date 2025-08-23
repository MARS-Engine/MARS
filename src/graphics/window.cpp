#include "mars/graphics/backend/window.hpp"
#include <mars/graphics/window.hpp>

#include <mars/graphics/graphics_engine.hpp>

namespace mars::graphics {
    window window_create(graphics_engine& _engine, const window_params& _params) {
        return _engine.get_impl<window_impl>().window_create(_engine, _params);
    }

    void window_get_extensions(const window& _window, std::vector<const char*>& _out) {
        return _window.engine->get_impl<window_impl>().get_extensions(_window, _out);
    }
} // namespace mars::graphics
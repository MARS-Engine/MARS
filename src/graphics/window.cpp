#include <mars/graphics/functional/window.hpp>

#include <mars/graphics/functional/graphics_engine.hpp>

namespace mars::graphics {
window window_create(graphics_engine& _engine, const window_params& _params) {
	return _engine.get_impl<window_impl>().window_create(_engine, _params);
}

void window_destroy(window& _window) {
	return _window.engine->get_impl<window_impl>().window_destroy(_window);
}
} // namespace mars::graphics

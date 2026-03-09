#pragma once

#include <string>

#include <mars/math/vector2.hpp>

namespace mars {
struct window;
struct instance;

struct graphics_engine;

struct window_params {
	std::string title;
	mars::vector2<size_t> size;
};

struct window_impl {
	window (*window_create)(graphics_engine& _engine, const window_params& _params) = nullptr;
	void (*window_destroy)(window& _window) = nullptr;
};
} // namespace mars

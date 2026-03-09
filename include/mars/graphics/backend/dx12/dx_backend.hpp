#pragma once

#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars::graphics {
struct directx_12_t {
	static graphics_backend_functions* get_functions();
};
} // namespace mars::graphics
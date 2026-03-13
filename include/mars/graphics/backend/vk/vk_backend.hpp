#pragma once

#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars::graphics {
struct vulkan_t {
	static graphics_backend_functions* get_functions();
};
} // namespace mars::graphics

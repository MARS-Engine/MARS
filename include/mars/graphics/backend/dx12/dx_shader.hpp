#pragma once

#include <mars/graphics/backend/shader.hpp>

namespace mars::graphics::dx {
struct dx_shader_impl {
	static shader dx_shader_create(const device& _device, const std::vector<shader_module>& _shaders);
	static void dx_shader_destroy(shader& _shader, const device& _device);
};
} // namespace mars::graphics::dx

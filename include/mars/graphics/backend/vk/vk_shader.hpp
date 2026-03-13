#pragma once

#include <mars/graphics/backend/shader.hpp>

namespace mars::graphics::vk {
struct vk_shader_impl {
	static shader vk_shader_create(const device& _device, const std::vector<shader_module>& _shaders);
	static void vk_shader_destroy(shader& _shader, const device& _device);
};
} // namespace mars::graphics::vk

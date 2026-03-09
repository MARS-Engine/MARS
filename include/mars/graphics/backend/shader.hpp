#pragma once

#include <mars/meta/type_erased.hpp>

#include <string>
#include <string_view>
#include <vector>

enum mars_shader_type {
	MARS_SHADER_TYPE_VERTEX,
	MARS_SHADER_TYPE_FRAGMENT,
	MARS_SHADER_TYPE_COMPUTE,
};

namespace mars {
struct graphics_backend_functions;
struct device;

struct shader {
	graphics_backend_functions* engine;
	meta::type_erased_ptr data;
	std::vector<std::string> semantics;
};

struct shader_module {
	mars_shader_type type;
	std::string_view path;
};

struct shader_impl {
	shader (*shader_create)(const device& _device, const std::vector<shader_module>& _shaders) = nullptr;
	void (*shader_destroy)(shader& _shader, const device& _device) = nullptr;
};
} // namespace mars

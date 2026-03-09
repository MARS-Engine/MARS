#pragma once

#include <mars/graphics/backend/shader.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>
#include <mars/meta.hpp>

#include <cctype>
#include <meta>

namespace mars {
namespace prop {
struct shader_annotation {
	mars_shader_type type;
};

struct shader_path_annotation {
	const char* path;
};

struct shader_input_annotation {
	mars_shader_type type;
};

inline static consteval shader_path_annotation shader(const char* _path) { return {std::define_static_string(_path)}; }
inline static consteval shader_input_annotation shader_input(mars_shader_type _type) { return {_type}; }
} // namespace prop

namespace graphics {

template <typename T>
inline shader shader_create(const device& _device) {
	constexpr auto ctx = std::meta::access_context::current();

	constexpr auto path_anno = mars::meta::get_annotation<prop::shader_path_annotation>(^^T);
	static_assert(path_anno.has_value(), "shader struct must have [[=mars::prop::shader(path)]] annotation");

	std::vector<shader_module> modules;
	std::vector<std::string> semantics;

	template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
		constexpr auto input_anno = mars::meta::get_annotation<prop::shader_input_annotation>(mem);
		if constexpr (input_anno.has_value()) {
			modules.push_back({.type = input_anno.value().type, .path = path_anno.value().path});

			if constexpr (input_anno.value().type == MARS_SHADER_TYPE_VERTEX) {
				using InputType = [:std::meta::type_of(mem):];
				template for (constexpr auto input_mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^InputType, ctx))) {
					std::string name(std::meta::identifier_of(input_mem));
					for (auto& c : name)
						c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
					semantics.push_back(std::move(name));
				}
			}
		}
	}

	auto result = _device.engine->get_impl<shader_impl>().shader_create(_device, modules);
	result.semantics = std::move(semantics);
	return result;
}

template <typename T>
inline shader shader_create_compute(const device& _device) {
	constexpr auto path_anno = mars::meta::get_annotation<prop::shader_path_annotation>(^^T);
	static_assert(path_anno.has_value(), "compute shader struct must have [[=mars::prop::shader(path)]] annotation");

	std::vector<shader_module> modules;
	modules.push_back({.type = MARS_SHADER_TYPE_COMPUTE, .path = path_anno.value().path});
	return _device.engine->get_impl<shader_impl>().shader_create(_device, modules);
}

inline void shader_destroy(shader& _shader, const device& _device) {
	return _shader.engine->get_impl<shader_impl>().shader_destroy(_shader, _device);
}
} // namespace graphics
} // namespace mars

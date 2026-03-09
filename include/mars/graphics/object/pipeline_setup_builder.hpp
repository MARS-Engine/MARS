#pragma once

#include <mars/graphics/backend/format.hpp>
#include <mars/graphics/backend/pipeline.hpp>

#include <cctype>
#include <meta>
#include <vector>

namespace mars::graphics::object {

struct pipeline_setup_builder {
	mars::pipeline_setup setup;

	pipeline_setup_builder& set_shader(const mars::shader& shader) {
		setup.pipeline_shader = shader;
		return *this;
	}

	template <typename T>
	pipeline_setup_builder& add_input_description(size_t binding = 0) {
		constexpr std::meta::access_context ctx = std::meta::access_context::current();
		constexpr size_t size = std::meta::nonstatic_data_members_of(^^T, ctx).size();

		mars::pipeline_binding_description description;
		description.binding = binding;
		description.stride = sizeof(T);

		setup.attributes.reserve(setup.attributes.size() + size);

		size_t location = 0;
		template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
			mars::pipeline_attribute_description attribute;
			attribute.location = location++;
			attribute.binding = binding;
			attribute.offset = std::meta::offset_of(mem).bytes;
			using C = [:std::meta::type_of(mem):];
			attribute.input_format = make_format<C>();

			std::string name(std::meta::identifier_of(mem));
			for (auto& c : name)
				c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
			attribute.semantic_name = std::move(name);
			setup.attributes.push_back(attribute);
		}

		setup.bindings.push_back(description);
		return *this;
	}

	pipeline_setup_builder& add_descriptor(mars::pipeline_descriptior_layout descriptor) {
		setup.descriptors.push_back(descriptor);
		return *this;
	}

	pipeline_setup_builder& set_push_constants(size_t count, mars_pipeline_stage stage) {
		setup.push_constant_count = count;
		setup.push_constant_stage = stage;
		return *this;
	}

	pipeline_setup_builder& set_depth_test(bool enabled) {
		setup.has_depth_test_override = true;
		setup.depth_test_enable = enabled;
		return *this;
	}

	pipeline_setup_builder& set_depth_write(bool enabled) {
		setup.has_depth_write_override = true;
		setup.depth_write_enable = enabled;
		return *this;
	}

	pipeline_setup_builder& set_depth_compare(mars_compare_op op) {
		setup.has_depth_compare_override = true;
		setup.depth_compare = op;
		return *this;
	}

	pipeline_setup_builder& set_alpha_blend(bool enabled) {
		setup.has_alpha_blend_override = true;
		setup.alpha_blend_enable = enabled;
		return *this;
	}

	mars::pipeline_setup build() const { return setup; }
};

} // namespace mars::graphics::object

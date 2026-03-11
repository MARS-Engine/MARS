#pragma once

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/compute_pipeline.hpp>
#include <mars/graphics/backend/descriptor.hpp>
#include <mars/graphics/functional/command_pool.hpp>
#include <mars/graphics/functional/compute_pipeline.hpp>
#include <mars/graphics/functional/descriptor.hpp>
#include <mars/graphics/functional/shader.hpp>
#include <mars/graphics/object/command_buffer.hpp>
#include <mars/graphics/object/pipeline_factory.hpp>
#include <mars/meta.hpp>

#include <cstddef>
#include <meta>
#include <type_traits>
#include <vector>

namespace mars {

template <typename T>
class compute_builder {
      public:
	struct compute_output_tex {
		texture tex = {};
		size_t mip_level = 0;
		size_t array_slice = 0;
	};

	struct inputs_type;
	struct settings_type;
	struct outputs_type;
	struct data_type;

	consteval {
		constexpr auto ctx = std::meta::access_context::current();

		std::vector<std::meta::info> in_fields, set_fields, out_fields;

		for (auto mem : std::meta::members_of(^^T, ctx)) {

			if (meta::has_annotation<graphics::compute_input>(mem))

				in_fields.push_back(
				    std::meta::data_member_spec(
					std::meta::type_of(mem),
					{.name = std::meta::identifier_of(mem)}));

			if (meta::has_annotation<graphics::compute_setting>(mem))

				set_fields.push_back(
				    std::meta::data_member_spec(
					^^buffer,
					{.name = std::meta::identifier_of(mem)}));

			if (meta::has_annotation<graphics::compute_output>(mem)) {
				if (std::meta::is_same_type(std::meta::type_of(mem), ^^texture))

					out_fields.push_back(
					    std::meta::data_member_spec(
						^^compute_output_tex,
						{.name = std::meta::identifier_of(mem)}));
				else

					out_fields.push_back(
					    std::meta::data_member_spec(
						^^buffer,
						{.name = std::meta::identifier_of(mem)}));
			}
		}

		std::meta::define_aggregate(^^inputs_type, in_fields);
		std::meta::define_aggregate(^^settings_type, set_fields);
		std::meta::define_aggregate(^^outputs_type, out_fields);

		std::meta::define_aggregate(^^data_type, {
							     std::meta::data_member_spec(^^inputs_type, {.name = "inputs"}),
							     std::meta::data_member_spec(^^settings_type, {.name = "settings"}),
							     std::meta::data_member_spec(^^outputs_type, {.name = "outputs"}),
							 });
	}

      private:
	compute_pipeline m_pipeline;
	descriptor m_descriptor;
	device m_device;
	descriptor_create_params m_pool_params;

      public:
	explicit compute_builder(const device& _device, size_t pool_size = 256)
	    : m_device(_device) {

		shader sh = graphics::shader_create_compute<T>(_device);

		compute_pipeline_setup setup;
		setup.pipeline_shader = sh;

		constexpr auto ctx = std::meta::access_context::current();

		template for (constexpr auto mem :
			      std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
			using C = typename[:std::meta::type_of(mem):];

			if constexpr (meta::has_annotation<graphics::compute_input>(mem)) {
				pipeline_descriptior_layout layout;
				layout.stage = MARS_PIPELINE_STAGE_COMPUTE;
				layout.binding = meta::get_annotation<graphics::compute_input>(mem).value().binding;

				if constexpr (std::is_same_v<C, texture>) {
					layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_IMAGE_SAMPLER;
					++m_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
				} else {
					layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					++m_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER];
				}
				setup.descriptors.push_back(layout);
			}

			if constexpr (meta::has_annotation<graphics::compute_setting>(mem)) {
				pipeline_descriptior_layout layout;
				layout.stage = MARS_PIPELINE_STAGE_COMPUTE;
				layout.binding = meta::get_annotation<graphics::compute_setting>(mem).value().binding;
				layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				++m_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER];
				setup.descriptors.push_back(layout);
			}

			if constexpr (meta::has_annotation<graphics::compute_output>(mem)) {
				pipeline_descriptior_layout layout;
				layout.stage = MARS_PIPELINE_STAGE_COMPUTE;
				layout.binding = meta::get_annotation<graphics::compute_output>(mem).value().binding;

				if constexpr (std::is_same_v<C, texture>) {
					layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					++m_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_STORAGE_IMAGE];
				} else {

					layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					++m_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_STORAGE_BUFFER];
				}
				setup.descriptors.push_back(layout);
			}
		}

		if constexpr (meta::has_annotation<graphics::push_constants_type>(^^T)) {
			constexpr auto pc = meta::get_annotation<graphics::push_constants_type>(^^T).value();
			setup.push_constant_count = pc.byte_count;
		} else if constexpr (meta::has_annotation<graphics::push_constants>(^^T)) {
			constexpr auto pc = meta::get_annotation<graphics::push_constants>(^^T).value();
			setup.push_constant_count = pc.count;
		}

		m_pipeline = graphics::compute_pipeline_create(_device, setup);

		descriptor_create_params pool_params = m_pool_params;
		for (size_t& s : pool_params.descriptors_size)
			s = (s > 0) ? s * pool_size : 0;
		pool_params.max_sets = pool_size;
		m_descriptor = mars::graphics::descriptor_create(_device, pool_params, 1);

		graphics::shader_destroy(sh, _device);
	}

	compute_builder(const compute_builder&) = delete;
	compute_builder& operator=(const compute_builder&) = delete;

	~compute_builder() { destroy(); }

	const compute_pipeline& get_pipeline() const { return m_pipeline; }

	void destroy() {
		if (m_descriptor.engine)
			mars::graphics::descriptor_destroy(m_descriptor, m_device);
		if (m_pipeline.engine)
			graphics::compute_pipeline_destroy(m_pipeline, m_device);
		m_descriptor = {};
		m_pipeline = {};
		m_device = {};
	}

	void dispatch(const command_buffer& cmd,
		      const data_type& data,
		      const command_buffer_dispatch_params& group_count) {
		const descriptor_set set = build_set(data);
		graphics::compute_pipeline_bind(m_pipeline, cmd);
		mars::graphics::descriptor_set_bind_compute(set, cmd, m_pipeline, 0);
		graphics::command_buffer_dispatch(cmd, group_count);
	}

	template <typename PC>
	void dispatch_pc(const command_buffer& cmd,
			 const data_type& data,
			 const PC& push_consts,
			 const command_buffer_dispatch_params& group_count) {
		const descriptor_set set = build_set(data);
		graphics::compute_pipeline_bind(m_pipeline, cmd);
		mars::graphics::descriptor_set_bind_compute(set, cmd, m_pipeline, 0);
		mars::graphics::object::command_buffer_set_compute_push_constants(cmd, m_pipeline, push_consts);
		graphics::command_buffer_dispatch(cmd, group_count);
	}

      private:
	descriptor_set build_set(const data_type& data) const {
		constexpr auto ctx = std::meta::access_context::current();

		descriptor_set_create_params params;

		template for (constexpr auto t_mem :
			      std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {

			if constexpr (meta::has_annotation<graphics::compute_input>(t_mem)) {
				constexpr size_t binding =
				    meta::get_annotation<graphics::compute_input>(t_mem).value().binding;

				template for (constexpr auto i_mem :
					      std::define_static_array(std::meta::nonstatic_data_members_of(^^inputs_type, ctx))) {
					if constexpr (std::meta::identifier_of(i_mem) == std::meta::identifier_of(t_mem)) {
						using IType = typename[:std::meta::type_of(i_mem):];
						if constexpr (std::is_same_v<IType, texture>)
							params.textures.push_back({
							    .image = data.inputs.[:i_mem:],
											  .binding = binding,
											  .descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_IMAGE_SAMPLER,
							});
						else
							params.buffers.emplace_back(data.inputs.[:i_mem:], binding);
					}
				}
			}

			if constexpr (meta::has_annotation<graphics::compute_setting>(t_mem)) {
				constexpr size_t binding =
				    meta::get_annotation<graphics::compute_setting>(t_mem).value().binding;

				template for (constexpr auto s_mem :
					      std::define_static_array(std::meta::nonstatic_data_members_of(^^settings_type, ctx))) {
					if constexpr (std::meta::identifier_of(s_mem) == std::meta::identifier_of(t_mem))
						params.buffers.emplace_back(data.settings.[:s_mem:], binding);
				}
			}

			if constexpr (meta::has_annotation<graphics::compute_output>(t_mem)) {
				constexpr size_t binding =
				    meta::get_annotation<graphics::compute_output>(t_mem).value().binding;

				template for (constexpr auto o_mem :
					      std::define_static_array(std::meta::nonstatic_data_members_of(^^outputs_type, ctx))) {
					if constexpr (std::meta::identifier_of(o_mem) == std::meta::identifier_of(t_mem)) {
						using OType = typename[:std::meta::type_of(o_mem):];
						if constexpr (std::is_same_v<OType, compute_output_tex>)
							params.textures.push_back({
							    .image = data.outputs.[:o_mem:]
								.tex,
								.binding = binding,
								.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
								.mip_level = data.outputs.
										  [:o_mem:]
								.mip_level,
								.array_slice = data.outputs.
										  [:o_mem:]
								.array_slice,
							});
						else
							params.buffers.emplace_back(data.outputs.[:o_mem:], binding);
					}
				}
			}
		}

		return mars::graphics::descriptor_set_create_compute(
		    m_descriptor, m_device, m_pipeline, {params});
	}
};

} // namespace mars

namespace mars {
template <typename T>
using compute_data = typename compute_builder<T>::data_type;
}

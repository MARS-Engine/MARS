#pragma once

#include <mars/graphics/backend/compute_pipeline.hpp>
#include <mars/graphics/functional/compute_pipeline.hpp>
#include <mars/graphics/object/pipeline_factory.hpp>
#include <mars/meta.hpp>

#include <meta>

namespace mars {

template <typename T>
class compute_pipeline_factory {
      private:
	compute_pipeline m_pipeline;
	device m_device;
	descriptor_create_params m_pool_params;

      public:
	compute_pipeline_factory(const device& _device, const shader& _shader) : m_device(_device) {
		compute_pipeline_setup setup;
		setup.pipeline_shader = _shader;

		constexpr std::meta::access_context ctx = std::meta::access_context::current();

		template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
			using C = typename[:std::meta::type_of(mem):];
			if constexpr (meta::has_annotation<graphics::uniform>(mem)) {
				pipeline_descriptior_layout layout;
				layout.stage = MARS_PIPELINE_STAGE_COMPUTE;
				layout.binding = meta::get_annotation<graphics::uniform>(mem).value().binding;

				constexpr bool is_uav = meta::has_annotation<graphics::writes_uav>(mem);
				if constexpr (std::is_same_v<C, texture>) {
					layout.descriptor_type = is_uav ? MARS_PIPELINE_DESCRIPTOR_TYPE_STORAGE_IMAGE
									: MARS_PIPELINE_DESCRIPTOR_TYPE_IMAGE_SAMPLER;
					++m_pool_params.descriptors_size[is_uav ? MARS_DESCRIPTOR_TYPE_STORAGE_IMAGE
										: MARS_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
				} else {
					layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					++m_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER];
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
	}

	const compute_pipeline& get_pipeline() const { return m_pipeline; }

	void destroy() {
		mars::graphics::compute_pipeline_destroy(m_pipeline, m_device);
	}

	descriptor_factory<T> create_descriptor(size_t _frames_in_flight, size_t _pool_size) {
		descriptor_create_params params = m_pool_params;
		for (size_t& entry : params.descriptors_size)
			entry = entry * _frames_in_flight * _pool_size;
		params.max_sets = _pool_size;
		descriptor desc = graphics::descriptor_create(m_device, params, _frames_in_flight);
		return descriptor_factory<T>(desc, m_device, m_pipeline);
	}
};

} // namespace mars

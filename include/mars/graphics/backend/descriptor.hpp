#pragma once

#include <mars/graphics/backend/compute_pipeline.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/pipeline.hpp>
#include <mars/graphics/backend/texture.hpp>

#include <array>
#include <meta>
#include <vector>

enum mars_descriptor_type {
	MARS_DESCRIPTOR_TYPE_SAMPLER,
	MARS_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	MARS_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	MARS_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	MARS_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
	MARS_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
	MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	MARS_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	MARS_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
	MARS_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
};

namespace mars {
struct descriptor {
	graphics_backend_functions* engine;
	meta::type_erased_ptr data;
	size_t frames_in_flight;
};

struct descriptor_create_params {
	std::array<size_t, std::meta::enumerators_of(^^mars_descriptor_type).size()> descriptors_size = {};
	size_t max_sets;
};

struct descriptor_set {
	graphics_backend_functions* engine;
	meta::type_erased_ptr data;
};

struct descriptor_set_create_params {
	std::vector<std::pair<buffer, size_t>> buffers;
	struct texture_binding {
		texture image;
		size_t binding;
		mars_pipeline_descriptor_type descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_IMAGE_SAMPLER;
		size_t mip_level = 0;
		size_t array_slice = 0;
	};
	std::vector<texture_binding> textures;
};

struct descriptor_impl {
	descriptor (*descriptor_create)(const device& _device, const descriptor_create_params& _params, size_t _frames_in_flight) = nullptr;
	descriptor_set (*descriptor_set_create)(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) = nullptr;
	descriptor_set (*descriptor_set_create_compute)(const descriptor& _descriptor, const device& _device, const compute_pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) = nullptr;
	void (*descriptor_set_bind)(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline& _pipeline, size_t _current_frame) = nullptr;
	void (*descriptor_set_bind_compute)(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const compute_pipeline& _pipeline, size_t _current_frame) = nullptr;
	void (*descriptor_set_update_cbv)(descriptor_set& _descriptor_set, size_t _binding, const buffer& _buffer) = nullptr;
	void (*descriptor_destroy)(descriptor& _descriptor, const device& _device) = nullptr;
};
} // namespace mars

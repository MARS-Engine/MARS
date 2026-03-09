#pragma once

#include <mars/graphics/backend/pipeline.hpp>

namespace mars {
struct command_buffer;

struct compute_pipeline {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
};

struct compute_pipeline_setup {
	shader pipeline_shader;
	std::vector<pipeline_descriptior_layout> descriptors;
	size_t push_constant_count = 0;
};

struct compute_pipeline_impl {
	compute_pipeline (*compute_pipeline_create)(const device& _device, const compute_pipeline_setup& _setup) = nullptr;
	void (*compute_pipeline_bind)(const compute_pipeline& _pipeline, const command_buffer& _command_buffer) = nullptr;
	void (*compute_pipeline_destroy)(compute_pipeline& _pipeline, const device& _device) = nullptr;
};
} // namespace mars

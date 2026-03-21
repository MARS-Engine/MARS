#pragma once

#include "append_buffer.hpp"
#include "buffer.hpp"
#include "command_pool.hpp"
#include "command_queue.hpp"
#include "compute_pipeline.hpp"
#include "descriptor.hpp"
#include "depth_buffer.hpp"
#include "device.hpp"
#include "framebuffer.hpp"
#include "indirect_executor.hpp"
#include "pipeline.hpp"
#include "readback_buffer.hpp"
#include "render_pass.hpp"
#include "shader.hpp"
#include "swapchain.hpp"
#include "texture.hpp"
#include "timeline_fence.hpp"
#include "upload_ring.hpp"
#include "window.hpp"

#include <mars/container/sparse_array.hpp>
#include <mars/graphics/functional/window.hpp>

#include <meta>
#include <utility>

namespace mars {
struct graphics_backend_functions {
	window_impl window;
	device_impl device;
	command_queue_impl command_queue;
	swapchain_impl swapchain;
	shader_impl shader;
	pipeline_impl pipeline;
	compute_pipeline_impl compute_pipeline;
	command_pool_impl command_pool;
	buffer_impl buffer;
	texture_impl texture;
	depth_buffer_impl depth_buffer;
	render_pass_impl render_pass;
	framebuffer_impl framebuffer;
	descriptor_impl descriptor;
	timeline_fence_impl timeline_fence;
	upload_ring_impl upload_ring;
	readback_buffer_impl readback_buffer;
	indirect_executor_impl indirect_executor;
	append_buffer_impl append_buffer;

	template <typename T>
	T& get_impl() {
		constexpr std::meta::access_context ctx = std::meta::access_context::current();
		template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^graphics_backend_functions, ctx))) {
			if constexpr (std::meta::type_of(mem) == ^^T)
				return this->[:mem:];
		}
		std::unreachable();
	}
};
}; // namespace mars

#pragma once

#include <mars/meta/type_erased.hpp>

namespace mars {
struct graphics_engine;
struct graphics_backend_functions;
struct command_buffer;
struct swapchain;
struct sync;

enum class device_feature : uint32_t {
	generated_commands,
};

struct device {
	graphics_backend_functions* engine;
	meta::type_erased_ptr data;
};

struct device_impl {
	device (*device_create)(graphics_engine& _engine) = nullptr;
	void (*device_submit)(const device& _device, const command_buffer& _command_buffer) = nullptr;
	void (*device_flush)(const device& _device) = nullptr;
	bool (*device_supports_feature)(const device& _device, device_feature feature) = nullptr;
	void (*device_destroy)(device& _device) = nullptr;
};
} // namespace mars

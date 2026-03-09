#pragma once

enum mars_command_queue_type {
	MARS_COMMAND_QUEUE_DIRECT = 0,
	MARS_COMMAND_QUEUE_COMPUTE = 1,
	MARS_COMMAND_QUEUE_COPY = 2,
};

namespace mars {
struct device;

struct command_queue_impl {
	void (*command_queue_create)(device& _device) = nullptr;
	void (*command_queue_destroy)(device& _device) = nullptr;
};
} // namespace mars

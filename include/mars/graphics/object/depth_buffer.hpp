#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/functional/depth_buffer.hpp>
#include <mars/graphics/object/command_recording.hpp>

namespace mars::graphics::object {

class depth_buffer {
  public:
	mars::depth_buffer_create_params params;

	depth_buffer() = default;

	explicit depth_buffer(const mars::depth_buffer_create_params& p) : params(p) {}

	depth_buffer(const mars::device& device, const mars::depth_buffer_create_params& p)
		: params(p) { create(device); }

	void create(const mars::device& device) {
		destroy();
		_device = device;
		_depth = mars::graphics::depth_buffer_create(_device, params);
	}

	void destroy() {
		if (!_depth.engine)
			return;

		mars::graphics::depth_buffer_destroy(_depth, _device);
		_depth = {};
		_device = {};
	}

	~depth_buffer() { destroy(); }

	depth_buffer(depth_buffer&& other) noexcept
		: params(other.params), _depth(other._depth), _device(other._device) {
		other._depth = {};
		other._device = {};
	}

	depth_buffer& operator=(depth_buffer&& other) noexcept {
		if (this != &other) {
			destroy();
			params = other.params;
			_depth = other._depth;
			_device = other._device;
			other._depth = {};
			other._device = {};
		}
		return *this;
	}

	depth_buffer(const depth_buffer&) = delete;
	depth_buffer& operator=(const depth_buffer&) = delete;

	void transition(command_buffer_recording& rec, mars_texture_state before, mars_texture_state after) {
		mars::graphics::depth_buffer_transition(rec.get(), _depth, before, after);
	}

	uint32_t get_srv_index() const {
		return mars::graphics::depth_buffer_get_srv_index(_depth);
	}

	const mars::depth_buffer& get() const { return _depth; }
	mars::depth_buffer& get() { return _depth; }

	operator const mars::depth_buffer&() const { return _depth; }
	operator mars::depth_buffer&() { return _depth; }

  private:
	mars::depth_buffer _depth = {};
	mars::device _device = {};
};

} // namespace mars::graphics::object

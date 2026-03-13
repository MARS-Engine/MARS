#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/functional/shader.hpp>

namespace mars::graphics::object {

template <typename T>
class shader {
  public:
	shader(const mars::device& device) : _device(device) {
		_shader = mars::graphics::shader_create<T>(_device);
	}

	~shader() {
		if (_shader.engine)
			mars::graphics::shader_destroy(_shader, _device);
	}

	shader(shader&& other) noexcept : _shader(other._shader), _device(other._device) {
		other._shader = {};
	}

	shader& operator=(shader&& other) noexcept {
		if (this != &other) {
			if (_shader.engine)
				mars::graphics::shader_destroy(_shader, _device);
			_shader = other._shader;
			_device = other._device;
			other._shader = {};
		}
		return *this;
	}

	shader(const shader&) = delete;
	shader& operator=(const shader&) = delete;

	const mars::shader& get() const {
		return _shader;
	}

	operator const mars::shader&() const {
		return _shader;
	}

  private:
	mars::shader _shader;
	mars::device _device;
};

} // namespace mars::graphics::object

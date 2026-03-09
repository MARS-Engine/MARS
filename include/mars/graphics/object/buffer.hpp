#pragma once
#include <cstring>
#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/functional/buffer.hpp>
#include <mars/graphics/object/pipeline_factory.hpp>

namespace mars::graphics::object {

inline uint32_t buffer_get_uav_index(const mars::buffer& buffer) {
	return buffer.engine->get_impl<mars::buffer_impl>().buffer_get_uav_index(buffer);
}

inline uint32_t buffer_get_srv_index(const mars::buffer& buffer) {
	return buffer.engine->get_impl<mars::buffer_impl>().buffer_get_srv_index(buffer);
}

inline void buffer_transition(const mars::command_buffer& command_buffer, const mars::buffer& buffer, mars_buffer_state state) {
	buffer.engine->get_impl<mars::buffer_impl>().buffer_transition(command_buffer, buffer, state);
}

template <typename T>
class buffer {
	mars::buffer m_buffer = {};
	mars::device m_device = {};
	size_t m_count = 0;

      public:
	buffer() = default;

	buffer(const mars::device& dev, uint32_t buffer_type, uint32_t buffer_property, size_t count = 1)
	    : m_device(dev), m_count(count) {
		m_buffer = mars::graphics::buffer_create(dev, {
								  .buffer_type = buffer_type,
								  .buffer_property = buffer_property,
								  .allocated_size = sizeof(T) * count,
								  .stride = sizeof(T),
							      });
	}

	~buffer() {
		if (m_buffer.engine)
			mars::graphics::buffer_destroy(m_buffer, m_device);
	}

	buffer(const buffer&) = delete;
	buffer& operator=(const buffer&) = delete;

	buffer(buffer&& o) noexcept
	    : m_buffer(o.m_buffer), m_device(o.m_device), m_count(o.m_count) { o.m_buffer = {}; }

	buffer& operator=(buffer&& o) noexcept {
		if (this != &o) {
			if (m_buffer.engine)
				mars::graphics::buffer_destroy(m_buffer, m_device);
			m_buffer = o.m_buffer;
			m_device = o.m_device;
			m_count = o.m_count;
			o.m_buffer = {};
		}
		return *this;
	}

	void upload(const T& val) {
		void* p = mars::graphics::buffer_map(m_buffer, m_device, sizeof(T), 0);
		std::memcpy(p, &val, sizeof(T));
		mars::graphics::buffer_unmap(m_buffer, m_device);
	}

	void upload(const T* data, size_t count) {
		void* p = mars::graphics::buffer_map(m_buffer, m_device, sizeof(T) * count, 0);
		std::memcpy(p, data, sizeof(T) * count);
		mars::graphics::buffer_unmap(m_buffer, m_device);
	}

	mars::mapped_buffer<T> map() {
		T* p = static_cast<T*>(
		    mars::graphics::buffer_map(m_buffer, m_device, sizeof(T) * m_count, 0));
		return mars::mapped_buffer<T>(p, &m_buffer, m_device);
	}

	size_t size() const { return m_count; }

	operator mars::buffer&() { return m_buffer; }
	operator const mars::buffer&() const { return m_buffer; }
	mars::buffer& get() { return m_buffer; }
	const mars::buffer& get() const { return m_buffer; }
	void transition(const mars::command_buffer& command_buffer, mars_buffer_state state) const {
		mars::graphics::object::buffer_transition(command_buffer, m_buffer, state);
	}
	uint32_t get_uav_index() const { return mars::graphics::object::buffer_get_uav_index(m_buffer); }
	uint32_t get_srv_index() const { return mars::graphics::object::buffer_get_srv_index(m_buffer); }
};

} // namespace mars::graphics::object

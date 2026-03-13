#pragma once

#include <mars/graphics/backend/append_buffer.hpp>
#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/functional/buffer.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

#include <cstdint>

namespace mars::graphics::object {

template <typename T>
class append_buffer {
	mars::append_buffer_base m_ab{};
	const mars::device* m_dev = nullptr;

  public:
	append_buffer() = default;

	static append_buffer create(const mars::device& dev, uint32_t capacity) {
		append_buffer obj;
		obj.m_ab = dev.engine->get_impl<append_buffer_impl>().append_buffer_create(dev, sizeof(T), capacity);
		obj.m_dev = &dev;
		return obj;
	}

	void reset_counter(const mars::command_buffer& cmd) const {
		m_ab.engine->get_impl<append_buffer_impl>().append_buffer_reset_counter(m_ab, cmd);
	}

	const mars::buffer& get_counter() const {
		return m_ab.engine->get_impl<append_buffer_impl>().append_buffer_get_counter_buffer(m_ab);
	}

	const mars::buffer& get_data() const {
		return m_ab.engine->get_impl<append_buffer_impl>().append_buffer_get_data_buffer(m_ab);
	}

	void transition_data(const mars::command_buffer& command_buffer, mars_buffer_state state) const {
		mars::graphics::buffer_transition(command_buffer, get_data(), state);
	}

	void transition_counter(const mars::command_buffer& command_buffer, mars_buffer_state state) const {
		mars::graphics::buffer_transition(command_buffer, get_counter(), state);
	}

	uint32_t get_data_uav_index() const { return mars::graphics::buffer_get_uav_index(get_data()); }
	uint32_t get_counter_uav_index() const { return mars::graphics::buffer_get_uav_index(get_counter()); }
	uint32_t get_data_srv_index() const { return mars::graphics::buffer_get_srv_index(get_data()); }
	uint32_t get_counter_srv_index() const { return mars::graphics::buffer_get_srv_index(get_counter()); }

	void destroy() {
		if (m_ab.engine && m_dev)
			m_ab.engine->get_impl<append_buffer_impl>().append_buffer_destroy(m_ab, *m_dev);

		m_ab.engine = nullptr;
		m_dev = nullptr;
	}

	~append_buffer() { destroy(); }

	append_buffer(const append_buffer&) = delete;
	append_buffer& operator=(const append_buffer&) = delete;

	append_buffer(append_buffer&& o) noexcept : m_ab(o.m_ab), m_dev(o.m_dev) {
		o.m_ab.engine = nullptr;
		o.m_dev = nullptr;
	}

	append_buffer& operator=(append_buffer&& o) noexcept {
		if (this != &o) {
			destroy();
			m_ab = o.m_ab;
			m_dev = o.m_dev;
			o.m_ab.engine = nullptr;
			o.m_dev = nullptr;
		}
		return *this;
	}

	mars::append_buffer_base& get() { return m_ab; }
	const mars::append_buffer_base& get() const { return m_ab; }
};

} // namespace mars::graphics::object

#pragma once

#include <mars/graphics/functional/command_pool.hpp>

#include <vector>

namespace mars::graphics::object {

class command_pool {
	mars::command_pool m_pool{};
	std::vector<command_buffer> m_buffers;
	mars::device m_device{};

  public:
	command_pool() = default;

	static command_pool create(const mars::device& device, size_t command_buffer_count = 0) {
		command_pool out;
		out.recreate(device, command_buffer_count);
		return out;
	}

	~command_pool() { destroy(); }

	command_pool(const command_pool&) = delete;
	command_pool& operator=(const command_pool&) = delete;

	command_pool(command_pool&& other) noexcept
		: m_pool(other.m_pool), m_buffers(std::move(other.m_buffers)), m_device(other.m_device) {
		other.m_pool = {};
	}

	command_pool& operator=(command_pool&& other) noexcept {
		if (this != &other) {
			destroy();
			m_pool = other.m_pool;
			m_buffers = std::move(other.m_buffers);
			m_device = other.m_device;
			other.m_pool = {};
		}
		return *this;
	}

	void recreate(const mars::device& device, size_t command_buffer_count = 0) {
		destroy();
		m_device = device;
		m_pool = mars::graphics::command_pool_create(m_device);
		if (command_buffer_count > 0)
			m_buffers = mars::graphics::command_buffer_create(m_pool, m_device, command_buffer_count);
	}

	void create_command_buffers(size_t count) {
		m_buffers = mars::graphics::command_buffer_create(m_pool, m_device, count);
	}

	command_buffer& buffer(size_t index) { return m_buffers.at(index); }
	const command_buffer& buffer(size_t index) const { return m_buffers.at(index); }

	std::vector<command_buffer>& buffers() { return m_buffers; }
	const std::vector<command_buffer>& buffers() const { return m_buffers; }

	size_t buffer_count() const { return m_buffers.size(); }

	const mars::command_pool& get() const { return m_pool; }
	mars::command_pool& get() { return m_pool; }

	void destroy() {
		m_buffers.clear();
		if (m_pool.engine)
			mars::graphics::command_pool_destroy(m_pool, m_device);
		m_pool = {};
	}
};

} // namespace mars::graphics::object

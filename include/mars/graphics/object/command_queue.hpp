#pragma once

#include <mars/graphics/functional/command_queue.hpp>

namespace mars::graphics::object {

class command_queue {
	mars::device* m_device = nullptr;
	bool m_created = false;

  public:
	command_queue() = default;

	static command_queue create(mars::device& device) {
		command_queue out;
		out.recreate(device);
		return out;
	}

	~command_queue() { destroy(); }

	command_queue(const command_queue&) = delete;
	command_queue& operator=(const command_queue&) = delete;

	command_queue(command_queue&& other) noexcept
		: m_device(other.m_device), m_created(other.m_created) {
		other.m_device = nullptr;
		other.m_created = false;
	}

	command_queue& operator=(command_queue&& other) noexcept {
		if (this != &other) {
			destroy();
			m_device = other.m_device;
			m_created = other.m_created;
			other.m_device = nullptr;
			other.m_created = false;
		}
		return *this;
	}

	void recreate(mars::device& device) {
		destroy();
		m_device = &device;
		mars::graphics::command_queue_create(*m_device);
		m_created = true;
	}

	void destroy() {
		if (m_created && m_device)
			mars::graphics::command_queue_destroy(*m_device);
		m_created = false;
		m_device = nullptr;
	}

	bool created() const { return m_created; }
};

} // namespace mars::graphics::object

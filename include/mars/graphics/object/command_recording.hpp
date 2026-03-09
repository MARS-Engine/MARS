#pragma once

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/functional/command_pool.hpp>
#include <mars/graphics/functional/device.hpp>
#include <mars/graphics/object/command_pool.hpp>

#include <string_view>

namespace mars::graphics::object {

class command_buffer_recording {
	mars::command_buffer* m_cmd = nullptr;
	const mars::device* m_device = nullptr;
	bool m_submitted = false;
	bool m_has_event = false;

      public:
	command_buffer_recording() = default;

	command_buffer_recording(mars::command_buffer& cmd, const mars::device& dev,
				 std::string_view name = {})
	    : m_cmd(&cmd), m_device(&dev) {
		mars::graphics::command_buffer_reset(cmd);
		mars::graphics::command_buffer_record(cmd);
		if (!name.empty()) {
			mars::graphics::command_buffer_begin_event(cmd, name);
			m_has_event = true;
		}
	}

	command_buffer_recording(mars::graphics::object::command_pool& pool,
				 const mars::device& dev,
				 std::string_view name = {},
				 size_t buffer_index = 0)
	    : command_buffer_recording(pool.buffer(buffer_index), dev, name) {}

	~command_buffer_recording() {
		if (!m_submitted && m_cmd)
			submit();
	}

	command_buffer_recording(const command_buffer_recording&) = delete;
	command_buffer_recording& operator=(const command_buffer_recording&) = delete;

	command_buffer_recording(command_buffer_recording&& o) noexcept
	    : m_cmd(o.m_cmd), m_device(o.m_device), m_submitted(o.m_submitted), m_has_event(o.m_has_event) { o.m_cmd = nullptr; }

	command_buffer_recording& operator=(command_buffer_recording&& o) noexcept {
		if (this != &o) {
			if (!m_submitted && m_cmd) submit();
			m_cmd = o.m_cmd;
			m_device = o.m_device;
			m_submitted = o.m_submitted;
			m_has_event = o.m_has_event;
			o.m_cmd = nullptr;
		}
		return *this;
	}

	void submit() {
		if (m_submitted || !m_cmd) return;
		if (m_has_event)
			mars::graphics::command_buffer_end_event(*m_cmd);
		mars::graphics::command_buffer_record_end(*m_cmd);
		mars::graphics::device_submit(*m_device, *m_cmd);
		m_submitted = true;
	}

	void flush() {
		submit();
		if (m_device) mars::graphics::device_flush(*m_device);
	}

	operator mars::command_buffer&() { return *m_cmd; }
	operator const mars::command_buffer&() const { return *m_cmd; }
	mars::command_buffer& get() { return *m_cmd; }
	const mars::command_buffer& get() const { return *m_cmd; }
};

} // namespace mars::graphics::object

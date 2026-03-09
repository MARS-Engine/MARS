#pragma once

#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/readback_buffer.hpp>

#include <cstddef>
#include <cstdint>

namespace mars::graphics::object {
class readback_buffer {
	mars::readback_buffer m_rb{};

      public:
	readback_buffer() = default;

	static readback_buffer create(const mars::device& dev, size_t slot_size, uint32_t num_slots) {
		readback_buffer obj;
		obj.m_rb = dev.engine->get_impl<readback_buffer_impl>().create(dev, slot_size, num_slots);
		return obj;
	}

	readback_ticket schedule(const mars::buffer& src, size_t src_offset, size_t size) {
		return m_rb.engine->get_impl<readback_buffer_impl>().schedule(m_rb, src, src_offset, size);
	}

	uint64_t flush() {
		return m_rb.engine->get_impl<readback_buffer_impl>().flush(m_rb);
	}

	bool try_read(const readback_ticket& ticket, const void** out_ptr) const {
		return m_rb.engine->get_impl<readback_buffer_impl>().try_read(m_rb, ticket, out_ptr);
	}

	void release_slot(const readback_ticket& ticket) {
		m_rb.engine->get_impl<readback_buffer_impl>().release_slot(m_rb, ticket);
	}

	void destroy() {
		if (m_rb.engine)
			m_rb.engine->get_impl<readback_buffer_impl>().destroy(m_rb);
		m_rb.engine = nullptr;
	}

	~readback_buffer() { destroy(); }

	readback_buffer(const readback_buffer&) = delete;
	readback_buffer& operator=(const readback_buffer&) = delete;

	readback_buffer(readback_buffer&& o) noexcept : m_rb(o.m_rb) {
		o.m_rb.engine = nullptr;
	}

	readback_buffer& operator=(readback_buffer&& o) noexcept {
		if (this != &o) {
			destroy();
			m_rb = o.m_rb;
			o.m_rb.engine = nullptr;
		}
		return *this;
	}

	mars::readback_buffer& get() { return m_rb; }
	const mars::readback_buffer& get() const { return m_rb; }
};
} // namespace mars::graphics::object

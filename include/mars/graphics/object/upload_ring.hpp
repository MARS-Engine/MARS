#pragma once

#include <cstddef>
#include <cstdint>
#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/upload_ring.hpp>

namespace mars::graphics::object {
class upload_ring {
	mars::upload_ring ring_{};
	const mars::device* dev_ = nullptr;

  public:
	upload_ring() = default;

	static upload_ring create(const mars::device& dev, size_t capacity = 64 * 1024 * 1024) {
		upload_ring r;
		r.dev_ = &dev;
		r.ring_ = dev.engine->get_impl<upload_ring_impl>().create(dev, capacity);
		return r;
	}

	void schedule(const mars::buffer& dst, size_t dst_offset, const void* src, size_t size) {
		ring_.engine->get_impl<upload_ring_impl>().schedule(ring_, dst, dst_offset, src, size);
	}

	uint64_t flush() {
		return ring_.engine->get_impl<upload_ring_impl>().flush(ring_);
	}

	void cpu_wait(uint64_t fence_value) {
		ring_.engine->get_impl<upload_ring_impl>().cpu_wait(ring_, fence_value);
	}

	void destroy() {
		if (ring_.engine)
			ring_.engine->get_impl<upload_ring_impl>().destroy(ring_);
		ring_ = {};
		dev_ = nullptr;
	}

	~upload_ring() {
		if (ring_.engine)
			ring_.engine->get_impl<upload_ring_impl>().destroy(ring_);
	}

	upload_ring(const upload_ring&) = delete;
	upload_ring& operator=(const upload_ring&) = delete;

	upload_ring(upload_ring&& o) noexcept : ring_(o.ring_), dev_(o.dev_) {
		o.ring_ = {};
		o.dev_ = nullptr;
	}

	upload_ring& operator=(upload_ring&& o) noexcept {
		if (this != &o) {
			if (ring_.engine)
				ring_.engine->get_impl<upload_ring_impl>().destroy(ring_);
			ring_ = o.ring_;
			dev_ = o.dev_;
			o.ring_ = {};
			o.dev_ = nullptr;
		}
		return *this;
	}
};
} // namespace mars::graphics::object

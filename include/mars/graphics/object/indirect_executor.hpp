#pragma once

#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/compute_pipeline.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/indirect_executor.hpp>
#include <mars/graphics/functional/indirect_executor.hpp>

#include <cstdint>

namespace mars::graphics::object {
class indirect_executor {
	mars::indirect_executor m_ex{};

  public:
	indirect_executor() = default;

	static indirect_executor create(const mars::device& dev, mars_command_signature_type type) {
		indirect_executor obj;
		obj.m_ex = mars::graphics::indirect_executor_create(dev, type);
		return obj;
	}

	static indirect_executor create_with_constant(const mars::device& dev, const mars::pipeline& pipe) {
		indirect_executor obj;
		obj.m_ex = mars::graphics::indirect_executor_create_with_constant(dev, pipe);
		return obj;
	}

	static indirect_executor create_with_constant(const mars::device& dev, const mars::compute_pipeline& pipe) {
		indirect_executor obj;
		obj.m_ex = mars::graphics::indirect_executor_create_with_constant(dev, pipe);
		return obj;
	}

	void record_dispatch(const mars::command_buffer& cmd, const mars::buffer& arg_buf, uint32_t max_count, const mars::buffer* count_buf = nullptr) const {
		mars::graphics::indirect_executor_record_dispatch(m_ex, cmd, arg_buf, max_count, count_buf);
	}

	void record_draw_indexed(const mars::command_buffer& cmd, const mars::buffer& arg_buf, uint32_t max_count, const mars::buffer* count_buf = nullptr) const {
		mars::graphics::indirect_executor_record_draw_indexed(m_ex, cmd, arg_buf, max_count, count_buf);
	}

	void destroy() {
		if (m_ex.engine)
			mars::graphics::indirect_executor_destroy(m_ex);
		m_ex.engine = nullptr;
	}

	~indirect_executor() { destroy(); }

	indirect_executor(const indirect_executor&) = delete;
	indirect_executor& operator=(const indirect_executor&) = delete;

	indirect_executor(indirect_executor&& o) noexcept : m_ex(o.m_ex) { o.m_ex.engine = nullptr; }
	indirect_executor& operator=(indirect_executor&& o) noexcept {
		if (this != &o) {
			destroy();
			m_ex = o.m_ex;
			o.m_ex.engine = nullptr;
		}
		return *this;
	}

	mars::indirect_executor& get() { return m_ex; }
	const mars::indirect_executor& get() const { return m_ex; }
};
} // namespace mars::graphics::object

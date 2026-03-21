#pragma once

#include <mars/graphics/backend/window.hpp>
#include <mars/graphics/functional/swapchain.hpp>

namespace mars::graphics::object {

class swapchain {
	mars::swapchain m_swapchain{};
	mars::device m_device{};

  public:
	swapchain() = default;

	static swapchain create(const mars::device& device, const mars::window& window, const mars::swapchain_create_params& params = {}) {
		swapchain out;
		out.m_device = device;
		out.m_swapchain = mars::graphics::swapchain_create(device, window, params);
		return out;
	}

	~swapchain() {
		if (m_swapchain.engine)
			mars::graphics::swapchain_destroy(m_swapchain, m_device);
	}

	swapchain(const swapchain&) = delete;
	swapchain& operator=(const swapchain&) = delete;

	swapchain(swapchain&& other) noexcept
		: m_swapchain(other.m_swapchain), m_device(other.m_device) {
		other.m_swapchain = {};
	}

	swapchain& operator=(swapchain&& other) noexcept {
		if (this != &other) {
			destroy();
			m_swapchain = other.m_swapchain;
			m_device = other.m_device;
			other.m_swapchain = {};
		}
		return *this;
	}

	void resize(const mars::vector2<size_t>& size) {
		mars::graphics::swapchain_resize(m_swapchain, m_device, size);
	}

	void present() const {
		mars::graphics::swapchain_present(m_swapchain, m_device);
	}

	size_t back_buffer_index() const {
		return mars::graphics::swapchain_get_back_buffer_index(m_swapchain);
	}

	size_t swapchain_size() const {
		return m_swapchain.swapchain_size;
	}

	mars::vector2<size_t> size() const {
		return m_swapchain.extent;
	}

	void destroy() {
		if (m_swapchain.engine)
			mars::graphics::swapchain_destroy(m_swapchain, m_device);
		m_swapchain = {};
	}

	const mars::swapchain& get() const { return m_swapchain; }
	mars::swapchain& get() { return m_swapchain; }

	mars_format_type format() const { return m_swapchain.format; }

	operator const mars::swapchain&() const { return m_swapchain; }
	operator mars::swapchain&() { return m_swapchain; }
};

} // namespace mars::graphics::object

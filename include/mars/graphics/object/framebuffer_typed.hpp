#pragma once

#include <mars/debug/logger.hpp>
#include <mars/graphics/functional/framebuffer.hpp>
#include <mars/graphics/functional/framebuffer.hpp>
#include <mars/graphics/object/pass_description.hpp>
#include <mars/graphics/object/schema.hpp>
#include <vector>

namespace mars::graphics::object {

template <typename PassTag>
struct framebuffer_handle {
	framebuffer value{};

	framebuffer_handle() = default;
	explicit framebuffer_handle(const framebuffer& fb) : value(fb) {}
};

template <typename PassTag>
struct swapchain_framebuffer_handle {
	framebuffer value{};

	swapchain_framebuffer_handle() = default;
	explicit swapchain_framebuffer_handle(const framebuffer& fb) : value(fb) {}
};

template <typename PassTag>
class framebuffer_resource {
	framebuffer_handle<PassTag> m_handle;
	device m_device{};

      public:
	static_assert(!pass_desc_traits<PassTag>::uses_swapchain,
		      "framebuffer_resource<PassTag>: PassTag uses swapchain; use swapchain_framebuffer_set instead");

	framebuffer_resource() = default;
	~framebuffer_resource() { destroy(); }

	framebuffer_resource(const framebuffer_resource&) = delete;
	framebuffer_resource& operator=(const framebuffer_resource&) = delete;

	framebuffer_resource(framebuffer_resource&& other) noexcept
	    : m_handle(other.m_handle), m_device(other.m_device) {
		other.m_handle.value = {};
		other.m_device = {};
	}

	framebuffer_resource& operator=(framebuffer_resource&& other) noexcept {
		if (this != &other) {
			destroy();
			m_handle = other.m_handle;
			m_device = other.m_device;
			other.m_handle.value = {};
			other.m_device = {};
		}
		return *this;
	}

	static framebuffer_resource create(const device& dev, const framebuffer_create_params& params) {
		framebuffer_resource out;
		out.recreate(dev, params);
		return out;
	}

	void recreate(const device& dev, const framebuffer_create_params& params) {
		destroy();
		m_device = dev;
		m_handle.value = mars::graphics::framebuffer_create(dev, params);
	}

	void destroy() {
		if (m_handle.value.engine)
			mars::graphics::framebuffer_destroy(m_handle.value, m_device);
		m_handle.value = {};
		m_device = {};
	}

	framebuffer_handle<PassTag>& get_handle() { return m_handle; }
	const framebuffer_handle<PassTag>& get_handle() const { return m_handle; }

	operator const framebuffer_handle<PassTag>&() const { return m_handle; }
};

template <typename PassTag>
class swapchain_framebuffer_set {
	std::vector<swapchain_framebuffer_handle<PassTag>> m_handles;
	device m_device{};

      public:
	static_assert(pass_desc_traits<PassTag>::uses_swapchain,
		      "swapchain_framebuffer_set<PassTag>: PassTag must be annotated with [[=mars::prop::rp_uses_swapchain()]]");

	swapchain_framebuffer_set() = default;
	~swapchain_framebuffer_set() { destroy_all(); }

	swapchain_framebuffer_set(const swapchain_framebuffer_set&) = delete;
	swapchain_framebuffer_set& operator=(const swapchain_framebuffer_set&) = delete;

	swapchain_framebuffer_set(swapchain_framebuffer_set&& other) noexcept
	    : m_handles(std::move(other.m_handles)), m_device(other.m_device) {
		other.m_device = {};
	}

	swapchain_framebuffer_set& operator=(swapchain_framebuffer_set&& other) noexcept {
		if (this != &other) {
			destroy_all();
			m_handles = std::move(other.m_handles);
			m_device = other.m_device;
			other.m_device = {};
		}
		return *this;
	}

	static swapchain_framebuffer_set create(
	    const device& dev,
	    const swapchain& sc,
	    const render_pass& rp) {
		swapchain_framebuffer_set out;
		out.recreate(dev, sc, rp);
		return out;
	}

	void recreate(
	    const device& dev,
	    const swapchain& sc,
	    const render_pass& rp) {
		destroy_all();
		m_device = dev;
		std::vector<framebuffer> raw = mars::graphics::framebuffer_create_from_swapchain(dev, sc, rp);
		m_handles.clear();
		m_handles.reserve(raw.size());
		for (const auto& fb : raw)
			m_handles.emplace_back(fb);
	}

	void destroy_all() {
		for (auto& fb : m_handles) {
			if (fb.value.engine)
				mars::graphics::framebuffer_destroy(fb.value, m_device);
			fb.value = {};
		}
		m_handles.clear();
		m_device = {};
	}

	size_t size() const { return m_handles.size(); }
	const swapchain_framebuffer_handle<PassTag>& operator[](size_t i) const { return m_handles.at(i); }
	swapchain_framebuffer_handle<PassTag>& operator[](size_t i) { return m_handles.at(i); }
};

} // namespace mars::graphics::object

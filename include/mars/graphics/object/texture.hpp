#pragma once

#include <cstring>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/functional/texture.hpp>
#include <mars/graphics/object/command_recording.hpp>

namespace mars::graphics::object {

inline uint32_t texture_get_srv_index(const mars::texture& texture) {
	return texture.engine->get_impl<mars::texture_impl>().texture_get_srv_index(texture);
}

inline uint32_t texture_get_uav_base(const mars::texture& texture) {
	return texture.engine->get_impl<mars::texture_impl>().texture_get_uav_base(texture);
}

inline void texture_transition(const mars::command_buffer& command_buffer,
			       mars::texture& texture,
			       mars_texture_state before,
			       mars_texture_state after) {
	texture.engine->get_impl<mars::texture_impl>().texture_transition(command_buffer, texture, before, after);
}

class mapped_texture {
	void* m_raw = nullptr;
	void* m_data = nullptr;
	texture_upload_layout m_layout = {};
	mars::texture* m_tex = nullptr;
	mars::device m_device = {};

      public:
	mapped_texture() = default;
	mapped_texture(mars::texture& tex, const mars::device& dev)
	    : m_tex(&tex), m_device(dev) {
		m_raw = mars::graphics::texture_map(tex, dev);
		m_layout = mars::graphics::texture_get_upload_layout(tex, dev);
		m_data = static_cast<uint8_t*>(m_raw) + m_layout.offset;
	}
	~mapped_texture() { unmap(); }

	mapped_texture(const mapped_texture&) = delete;
	mapped_texture& operator=(const mapped_texture&) = delete;

	mapped_texture(mapped_texture&& o) noexcept
	    : m_raw(o.m_raw), m_data(o.m_data), m_layout(o.m_layout), m_tex(o.m_tex), m_device(o.m_device) { o.m_raw = nullptr; }

	mapped_texture& operator=(mapped_texture&& o) noexcept {
		if (this != &o) {
			unmap();
			m_raw = o.m_raw;
			m_data = o.m_data;
			m_layout = o.m_layout;
			m_tex = o.m_tex;
			m_device = o.m_device;
			o.m_raw = nullptr;
		}
		return *this;
	}

	void* operator*() { return m_data; }

	const texture_upload_layout& layout() const { return m_layout; }

	void unmap() {
		if (m_raw) {
			mars::graphics::texture_unmap(*m_tex, m_device);
			m_raw = nullptr;
		}
	}
};

class texture {
      public:
	mars::texture_create_params params;

	texture() = default;

	explicit texture(const mars::texture_create_params& p) : params(p) {}

	texture(const mars::device& device, const mars::texture_create_params& p)
	    : params(p) { create(device); }

	void create(const mars::device& device) {
		_device = device;
		_texture = mars::graphics::texture_create(_device, params);
	}

	~texture() {
		if (_texture.engine)
			mars::graphics::texture_destroy(_texture, _device);
	}

	texture(texture&& other) noexcept : _texture(other._texture), _device(other._device) {
		other._texture = {};
	}

	texture& operator=(texture&& other) noexcept {
		if (this != &other) {
			if (_texture.engine)
				mars::graphics::texture_destroy(_texture, _device);
			_texture = other._texture;
			_device = other._device;
			other._texture = {};
		}
		return *this;
	}

	texture(const texture&) = delete;
	texture& operator=(const texture&) = delete;

	void upload_data(const void* data) {
		auto layout = mars::graphics::texture_get_upload_layout(_texture, _device);
		void* mapped_ptr = mars::graphics::texture_map(_texture, _device);

		if (mapped_ptr) {
			char* mapped = static_cast<char*>(mapped_ptr) + layout.offset;
			const char* src = static_cast<const char*>(data);

			if (layout.row_pitch == layout.row_size)
				std::memcpy(mapped, src, layout.row_count * layout.row_size);
			else
				for (size_t y = 0; y < layout.row_count; ++y)
					std::memcpy(mapped + y * layout.row_pitch,
						    src + y * layout.row_size,
						    layout.row_size);
			mars::graphics::texture_unmap(_texture, _device);
		}
	}

	template <typename Container>
	void upload_data(const Container& container) {
		upload_data(container.data());
	}

	mapped_texture map() {
		return mapped_texture(_texture, _device);
	}

	const mars::texture& get() const {
		return _texture;
	}

	mars::texture& get() {
		return _texture;
	}

	uint32_t get_srv_index() const {
		return mars::graphics::object::texture_get_srv_index(_texture);
	}

	uint32_t get_uav_base() const {
		return mars::graphics::object::texture_get_uav_base(_texture);
	}

	operator const mars::texture&() const {
		return _texture;
	}

	operator mars::texture&() {
		return _texture;
	}

	void copy_to_gpu(command_buffer_recording& rec) {
		mars::buffer dummy = {};
		mars::graphics::texture_copy(_texture, dummy, rec.get(), 0);
	}

	void transition(command_buffer_recording& rec,
			mars_texture_state before, mars_texture_state after) {
		mars::graphics::object::texture_transition(rec.get(), _texture, before, after);
	}

      private:
	mars::texture _texture = {};
	mars::device _device = {};
};

} // namespace mars::graphics::object

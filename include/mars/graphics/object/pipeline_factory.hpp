#pragma once

#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/compute_pipeline.hpp>
#include <mars/graphics/backend/descriptor.hpp>
#include <mars/graphics/backend/pipeline.hpp>
#include <mars/graphics/backend/render_pass.hpp>
#include <mars/graphics/functional/buffer.hpp>
#include <mars/graphics/functional/command_pool.hpp>
#include <mars/graphics/functional/descriptor.hpp>
#include <mars/graphics/functional/device.hpp>
#include <mars/graphics/functional/render_pass.hpp>
#include <mars/graphics/functional/shader.hpp>
#include <mars/graphics/object/command_recording.hpp>
#include <mars/graphics/object/pipeline.hpp>
#include <mars/graphics/object/pipeline_setup_builder.hpp>
#include <mars/graphics/object/schema.hpp>
#include <mars/meta.hpp>

#include <cstdint>
#include <cstring>
#include <meta>
#include <type_traits>
#include <variant>
#include <vector>

namespace mars {

template <typename T>
class mapped_buffer {
	T* m_ptr = nullptr;
	buffer* m_buf = nullptr;
	device m_device = {};

  public:
	mapped_buffer() = default;
	mapped_buffer(T* ptr, buffer* buf, device dev)
		: m_ptr(ptr), m_buf(buf), m_device(dev) {}
	mapped_buffer(const mapped_buffer&) = delete;
	mapped_buffer& operator=(const mapped_buffer&) = delete;
	mapped_buffer(mapped_buffer&& o) noexcept
		: m_ptr(o.m_ptr), m_buf(o.m_buf), m_device(o.m_device) { o.m_ptr = nullptr; }
	mapped_buffer& operator=(mapped_buffer&& o) noexcept {
		if (this != &o) {
			unmap();
			m_ptr = o.m_ptr;
			m_buf = o.m_buf;
			m_device = o.m_device;
			o.m_ptr = nullptr;
		}
		return *this;
	}
	~mapped_buffer() { unmap(); }
	T* operator->() { return m_ptr; }
	T& operator*() { return *m_ptr; }
	T* data() { return m_ptr; }
	const T* data() const { return m_ptr; }
	T& operator[](size_t i) { return m_ptr[i]; }
	const T& operator[](size_t i) const { return m_ptr[i]; }
	void unmap() {
		if (m_ptr) {
			mars::graphics::buffer_unmap(*m_buf, m_device);
			m_ptr = nullptr;
		}
	}
};

template <typename LayoutT>
struct owned_descriptor_set {
	descriptor_set set;
	std::vector<buffer> owned_buffers;
	device m_device;

	struct mapped_data;
	consteval {
		auto ctx = std::meta::access_context::current();
		std::vector<std::meta::info> fields;
		for (auto mem : std::meta::nonstatic_data_members_of(^^LayoutT, ctx)) {
			if (!meta::has_annotation<graphics::uniform>(mem)) continue;
			if (std::meta::is_same_type(std::meta::type_of(mem), ^^buffer)) continue;
			auto mb_type = std::meta::substitute(^^mapped_buffer, {std::meta::type_of(mem)});
			fields.push_back(std::meta::data_member_spec(mb_type, {.name = std::meta::identifier_of(mem)}));
		}
		std::meta::define_aggregate(^^mapped_data, fields);
	}

	owned_descriptor_set() = default;
	owned_descriptor_set(const owned_descriptor_set&) = delete;
	owned_descriptor_set& operator=(const owned_descriptor_set&) = delete;

	owned_descriptor_set(owned_descriptor_set&& o) noexcept
		: set(std::move(o.set)), owned_buffers(std::move(o.owned_buffers)), m_device(o.m_device) {}

	~owned_descriptor_set() { destroy(); }

	owned_descriptor_set& operator=(owned_descriptor_set&& o) noexcept {
		if (this != &o) {
			destroy();
			set = std::move(o.set);
			owned_buffers = std::move(o.owned_buffers);
			m_device = o.m_device;
		}
		return *this;
	}

	operator const descriptor_set&() const { return set; }

	void bind(const mars::graphics::object::command_buffer_recording& rec, const pipeline& pl, size_t frame = 0) {
		mars::graphics::descriptor_set_bind(set, rec.get(), pl, frame);
	}

	void bind(const mars::graphics::object::command_buffer_recording& rec, const compute_pipeline& pl, size_t frame = 0) {
		mars::graphics::descriptor_set_bind_compute(set, rec.get(), pl, frame);
	}

	mapped_data get() {
		mapped_data result;
		constexpr auto ctx = std::meta::access_context::current();
		size_t idx = 0;
		template for (constexpr auto layout_mem : std::define_static_array(
						  std::meta::nonstatic_data_members_of(^^LayoutT, ctx)
					  )) {
			if constexpr (meta::has_annotation<graphics::uniform>(layout_mem) &&
						  !std::meta::is_same_type(std::meta::type_of(layout_mem), ^^buffer)) {
				using MT = typename[:std::meta::type_of(layout_mem):];
				void* raw = mars::graphics::buffer_map(owned_buffers[idx], m_device, sizeof(MT), 0);
				template for (constexpr auto data_mem : std::define_static_array(
								  std::meta::nonstatic_data_members_of(^^mapped_data, ctx)
							  )) {
					if constexpr (std::meta::identifier_of(data_mem) == std::meta::identifier_of(layout_mem))
						result.[:data_mem:] = mapped_buffer<MT>(static_cast<MT*>(raw), &owned_buffers[idx], m_device);
				}
				++idx;
			}
		}
		return result;
	}

	template <std::meta::info Member>
	void set_buffer(const buffer& _buffer) {
		constexpr size_t binding = []() consteval -> size_t {
			constexpr auto ctx = std::meta::access_context::current();
			for (auto mem : std::meta::nonstatic_data_members_of(^^LayoutT, ctx)) {
				if (!meta::has_annotation<graphics::uniform>(mem)) continue;
				if (!std::meta::is_same_type(std::meta::type_of(mem), ^^buffer)) continue;
				if (mem == Member)
					return meta::get_annotation<graphics::uniform>(mem).value().binding;
			}
			return ~size_t(0);
		}();
		static_assert(binding != ~size_t(0), "set_buffer<>: Member is not an explicit mars::buffer field in LayoutT");
		mars::graphics::descriptor_set_update_cbv(set, binding, _buffer);
	}

	void destroy() {
		for (auto& b : owned_buffers)
			mars::graphics::buffer_destroy(b, m_device);
		owned_buffers.clear();
	}
};

template <typename T>
class descriptor_factory {
  private:
	device m_device;
	descriptor m_descriptor;
	std::variant<pipeline, compute_pipeline> m_pipeline;

  public:
	descriptor_factory(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline)
		: m_descriptor(_descriptor), m_device(_device), m_pipeline(_pipeline) {}

	descriptor_factory(const descriptor& _descriptor, const device& _device, const compute_pipeline& _pipeline)
		: m_descriptor(_descriptor), m_device(_device), m_pipeline(_pipeline) {}

	void destroy() {
		mars::graphics::descriptor_destroy(m_descriptor, m_device);
	}

	owned_descriptor_set<T> allocate_set() {
		constexpr auto ctx = std::meta::access_context::current();

		mars::descriptor_set_create_params params;
		owned_descriptor_set<T> result;
		result.m_device = m_device;

		consteval {
		}

		template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
			if constexpr (meta::has_annotation<graphics::uniform>(mem)) {
				using MT = typename[:std::meta::type_of(mem):];
				constexpr size_t binding = meta::get_annotation<graphics::uniform>(mem).value().binding;

				if constexpr (!std::is_same_v<MT, buffer>) {
					constexpr size_t sz = sizeof(MT);
					buffer auto_buf = mars::graphics::buffer_create(m_device, {
																				  .buffer_type = MARS_BUFFER_TYPE_UNIFORM,
																				  .buffer_property = MARS_BUFFER_PROPERTY_HOST_VISIBLE,
																				  .allocated_size = sz,
																			  });
					params.buffers.emplace_back(auto_buf, binding);
					result.owned_buffers.push_back(auto_buf);
				}
			}
		}

		std::vector<mars::descriptor_set_create_params> set_params = {std::move(params)};

		result.set = std::visit([&](const auto& pl) -> descriptor_set {
			using PL = std::decay_t<decltype(pl)>;
			if constexpr (std::is_same_v<PL, pipeline>)
				return mars::graphics::descriptor_set_create(m_descriptor, m_device, pl, set_params);
			else
				return mars::graphics::descriptor_set_create_compute(m_descriptor, m_device, pl, set_params);
		},
								m_pipeline);
		return result;
	}
};

template <typename T>
class mesh_data {
  private:
	std::vector<buffer> m_buffers;
	buffer m_indice_buffer;
	size_t m_index_count = 0;
	device m_device;

  public:
	mesh_data() = default;
	mesh_data(const device& _device, const std::vector<buffer>& _buffers, const buffer& _indice, size_t _index_count)
		: m_buffers(_buffers), m_indice_buffer(_indice), m_index_count(_index_count), m_device(_device) {}

	void upload(buffer& _buffer, const void* data, size_t size) {
		void* mapped = mars::graphics::buffer_map(_buffer, m_device, size, 0);
		std::memcpy(mapped, data, size);
		mars::graphics::buffer_unmap(_buffer, m_device);
	}

	void upload_vertices(const void* data, size_t size, size_t _buffer_index = 0) {
		upload(m_buffers.at(_buffer_index), data, size);
	}

	void upload_indices(const void* data, size_t size) {
		upload(m_indice_buffer, data, size);
	}

	void destroy() {
		for (buffer& entry : m_buffers)
			mars::graphics::buffer_destroy(entry, m_device);
		if (m_indice_buffer.engine)
			mars::graphics::buffer_destroy(m_indice_buffer, m_device);
		m_buffers.clear();
		m_indice_buffer = {};
	}

	std::vector<buffer>& get_buffers() { return m_buffers; }
	const std::vector<buffer>& get_buffers() const { return m_buffers; }
	buffer& get_index_buffer() { return m_indice_buffer; }
	const buffer& get_index_buffer() const { return m_indice_buffer; }
	size_t get_index_count() const { return m_index_count; }
};

template <typename T>
class pipeline_factory {
  private:
	pipeline m_pipeline;
	device m_device;
	mars::descriptor_create_params descriptor_pool_params;

  public:
	pipeline_factory(const device& _device, const shader& _shader, const render_pass& _render_pass) : m_device(_device) {
		mars::graphics::object::pipeline_setup_builder setup;
		setup.set_shader(_shader);

		constexpr std::meta::access_context ctx = std::meta::access_context::current();

		template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
			using C = typename[:std::meta::type_of(mem):];
			if constexpr (meta::has_annotation<graphics::input>(mem))
				setup.add_input_description<C>(meta::get_annotation<graphics::input>(mem).value().binding);
			else if constexpr (meta::has_annotation<graphics::uniform>(mem)) {
				pipeline_descriptior_layout layout;
				layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				++descriptor_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER];

				layout.stage = meta::get_annotation<graphics::uniform>(mem).value().stage;
				layout.binding = meta::get_annotation<graphics::uniform>(mem).value().binding;
				setup.add_descriptor(layout);
			}
		}

		if constexpr (meta::has_annotation<graphics::push_constants_type>(^^T)) {
			constexpr auto pc = meta::get_annotation<graphics::push_constants_type>(^^T).value();
			setup.set_push_constants(pc.byte_count, pc.stage);
		} else if constexpr (meta::has_annotation<graphics::push_constants>(^^T)) {
			constexpr auto pc = meta::get_annotation<graphics::push_constants>(^^T).value();
			setup.set_push_constants(pc.count, pc.stage);
		}

		if constexpr (meta::has_annotation<graphics::depth_test>(^^T)) {
			constexpr auto depth_test = meta::get_annotation<graphics::depth_test>(^^T).value();
			setup.set_depth_test(depth_test.enabled);
		}

		if constexpr (meta::has_annotation<graphics::depth_write>(^^T)) {
			constexpr auto depth_write = meta::get_annotation<graphics::depth_write>(^^T).value();
			setup.set_depth_write(depth_write.enabled);
		}

		if constexpr (meta::has_annotation<graphics::depth_compare>(^^T)) {
			constexpr auto depth_compare = meta::get_annotation<graphics::depth_compare>(^^T).value();
			setup.set_depth_compare(depth_compare.op);
		}

		if constexpr (meta::has_annotation<graphics::blend_alpha>(^^T)) {
			constexpr auto alpha_blend = meta::get_annotation<graphics::blend_alpha>(^^T).value();
			setup.set_alpha_blend(alpha_blend.enabled);
		}

		m_pipeline = graphics::object::pipeline_create(_device, _render_pass, setup.build());
	}

	const pipeline& get_pipeline() const { return m_pipeline; }

	void destroy() {
		mars::graphics::object::pipeline_destroy(m_pipeline, m_device);
	}

	descriptor_factory<T> create_descriptor(size_t _frames_in_flight, size_t _pool_size) {
		mars::descriptor_create_params params = descriptor_pool_params;
		for (size_t& entry : params.descriptors_size)
			entry = entry * _frames_in_flight * _pool_size;
		params.max_sets = _pool_size;
		mars::descriptor descriptor = mars::graphics::descriptor_create(m_device, params, _frames_in_flight);
		return descriptor_factory<T>(descriptor, m_device, m_pipeline);
	}

	mesh_data<T> create_input_buffers(size_t _vertex_count, size_t _index_count = 0) {
		constexpr std::meta::access_context ctx = std::meta::access_context::current();

		std::vector<buffer> result;

		template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
			if constexpr (meta::has_annotation<graphics::input>(mem)) {
				buffer new_buffer = mars::graphics::buffer_create(m_device, {.buffer_type = MARS_BUFFER_TYPE_VERTEX | MARS_BUFFER_TYPE_TRANSFER_DST, .buffer_property = MARS_BUFFER_PROPERTY_HOST_VISIBLE, .allocated_size = _vertex_count * std::meta::size_of(mem), .stride = std::meta::size_of(mem)});
				result.push_back(new_buffer);
			}
		}

		buffer indice_buffer;

		if (_index_count > 0)
			indice_buffer = mars::graphics::buffer_create(m_device, {.buffer_type = MARS_BUFFER_TYPE_INDEX | MARS_BUFFER_TYPE_TRANSFER_DST, .buffer_property = MARS_BUFFER_PROPERTY_HOST_VISIBLE, .allocated_size = _index_count * sizeof(uint32_t)});

		return mesh_data<T>(m_device, result, indice_buffer, _index_count);
	}
};

} // namespace mars

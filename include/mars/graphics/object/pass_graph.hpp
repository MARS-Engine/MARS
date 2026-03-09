#pragma once

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/functional/command_pool.hpp>
#include <mars/graphics/functional/compute_pipeline.hpp>
#include <mars/graphics/functional/pipeline.hpp>
#include <mars/graphics/object/pass_scope.hpp>
#include <mars/graphics/object/pipeline_factory.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta.hpp>

#include <meta>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace mars {
namespace detail {

template <typename T>
inline const char type_anchor = 0;

template <typename T>
size_t type_id() noexcept {
	return reinterpret_cast<size_t>(&type_anchor<T>);
}
} // namespace detail

template <typename... Nodes>
class pass_graph {
	static_assert(sizeof...(Nodes) > 0, "pass_graph must have at least one node");

      private:
	std::tuple<Nodes*...> m_node_ptrs;
	device m_device;

	std::unordered_map<size_t, std::pair<mars::texture*, mars_texture_state>> m_textures;

      public:
	pass_graph(const device& dev, Nodes&... nodes)
	    : m_device(dev), m_node_ptrs(std::make_tuple(&nodes...)) {}

	~pass_graph() { destroy(); }

	pass_graph(const pass_graph&) = delete;
	pass_graph& operator=(const pass_graph&) = delete;

	template <typename ProducerTag>
	void register_texture(mars::texture& tex,
			      mars_texture_state initial_state = MARS_TEXTURE_STATE_SHADER_READ) {
		m_textures[detail::type_id<ProducerTag>()] = {&tex, initial_state};
	}

	void create_descriptors(size_t frames_in_flight, size_t pool_size) {
		std::apply([&](auto*... ptrs) {
			(ptrs->create_descriptors(frames_in_flight, pool_size), ...);
		},
			   m_node_ptrs);
	}

	void destroy() {
		if (!m_device.engine)
			return;
		std::apply([&](auto*... ptrs) {
			(ptrs->destroy(), ...);
		},
			   m_node_ptrs);
		m_textures.clear();
		m_device = {};
	}

	template <typename... Fns>
	void execute(command_buffer& cmd, vector2<size_t> window_size, Fns&&... fns) {
		static_assert(sizeof...(Fns) == sizeof...(Nodes),
			      "execute() must receive exactly one callback per node");
		auto fn_tuple = std::forward_as_tuple(std::forward<Fns>(fns)...);
		[this, &cmd, &window_size, &fn_tuple]<size_t... Is>(std::index_sequence<Is...>) {
			(execute_node<Is>(cmd, window_size, std::get<Is>(fn_tuple)), ...);
		}(std::make_index_sequence<sizeof...(Nodes)>{});
	}

      private:
	template <size_t I, typename Fn>
	void execute_node(command_buffer& cmd, vector2<size_t> window_size, Fn&& fn) {
		using NodePtrT = std::tuple_element_t<I, std::tuple<Nodes*...>>;
		using NodeT = std::remove_pointer_t<NodePtrT>;
		using LayoutT = typename NodeT::layout_type;
		using TagT = typename NodeT::tag_type;

		auto* node_ptr = std::get<I>(m_node_ptrs);

		[this, &cmd]<size_t... Js>(std::index_sequence<Js...>) {
			(insert_read_barriers_for_producer<Js, LayoutT>(cmd), ...);
		}(std::make_index_sequence<I>{});

		if constexpr (NodeT::is_compute) {
			auto it = m_textures.find(detail::type_id<TagT>());
			if (it != m_textures.end()) {
				auto& [tex_ptr, cur_state] = it->second;
				if (cur_state != MARS_TEXTURE_STATE_UNORDERED_ACCESS) {
					graphics::texture_transition(
					    cmd, *tex_ptr, cur_state, MARS_TEXTURE_STATE_UNORDERED_ACCESS);
					cur_state = MARS_TEXTURE_STATE_UNORDERED_ACCESS;
				}
			}
		}

		if constexpr (!NodeT::is_compute) {

			vector2<size_t> pass_size = window_size;
			if constexpr (meta::has_annotation<graphics::rp_size>(^^typename NodeT::tag_type)) {
				constexpr auto sz = meta::get_annotation<graphics::rp_size>(^^typename NodeT::tag_type).value();
				if constexpr (sz.width != 0)
					pass_size = {sz.width, sz.height};
			}
			node_ptr->bind(cmd, pass_size);
		} else {
			node_ptr->bind(cmd);
		}

		constexpr std::string_view pass_name = std::meta::display_string_of(std::meta::decay(^^TagT));
		mars::graphics::command_buffer_begin_event(cmd, pass_name);
		if constexpr (NodeT::is_compute) {
			compute_scope scope(cmd);
			std::forward<Fn>(fn)(*node_ptr, scope);
		} else {
			raster_scope<typename NodeT::tag_type> scope(cmd, node_ptr->get_render_pass(), window_size);
			std::forward<Fn>(fn)(*node_ptr, scope);
		}
		mars::graphics::command_buffer_end_event(cmd);
	}

	template <size_t ProducerIdx, typename ConsumerLayoutT>
	void insert_read_barriers_for_producer(command_buffer& cmd) {
		using ProducerNodeT = std::remove_pointer_t<
		    std::tuple_element_t<ProducerIdx, std::tuple<Nodes*...>>>;
		using ProducerTag = typename ProducerNodeT::tag_type;

		constexpr auto ctx = std::meta::access_context::current();
		template for (constexpr auto mem :
			      std::define_static_array(
				  std::meta::nonstatic_data_members_of(^^ConsumerLayoutT, ctx))) {
			if constexpr (meta::has_annotation<graphics::reads_from<ProducerTag>>(mem)) {
				constexpr auto ann =
				    meta::get_annotation<graphics::reads_from<ProducerTag>>(mem).value();
				auto it = m_textures.find(detail::type_id<ProducerTag>());
				if (it != m_textures.end()) {
					auto& [tex_ptr, cur_state] = it->second;
					if (cur_state != ann.required_state) {
						graphics::texture_transition(
						    cmd, *tex_ptr, cur_state, ann.required_state);
						cur_state = ann.required_state;
					}
				}
			}
		}
	}
};

} // namespace mars

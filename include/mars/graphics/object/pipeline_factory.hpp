#pragma once

#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/descriptor.hpp>
#include <mars/graphics/backend/pipeline.hpp>
#include <mars/graphics/functional/buffer.hpp>
#include <mars/graphics/functional/descriptor.hpp>
#include <mars/graphics/functional/device.hpp>
#include <mars/graphics/functional/pipeline.hpp>
#include <mars/graphics/functional/render_pass.hpp>
#include <mars/graphics/functional/shader.hpp>
#include <mars/graphics/functional/staging_buffer.hpp>
#include <mars/meta.hpp>

#include <cstdint>
#include <meta>
#include <type_traits>
#include <vector>

namespace mars {
    namespace graphics {
        struct input {
            size_t binding;
            mars_pipeline_stage stage;
        };

        struct uniform {
            size_t binding;
            mars_pipeline_stage stage;
        };
    } // namespace graphics

    template <typename T>
    class descriptor_factory {
      private:
        device m_device;
        descriptor m_descriptor;
        pipeline m_pipeline;

      public:
        template <typename C, size_t B>
        struct binding_wrapper {
            typedef C type;
            inline constexpr static size_t binding = B;
            C value;

            binding_wrapper& operator=(const C& _other) {
                value = _other;
                return *this;
            }
        };

        struct data_type;
        consteval {
            std::vector<std::meta::info> result = {};

            auto ctx = std::meta::access_context::current();

            for (auto mem : std::meta::members_of(^^T, ctx)) {
                if (meta::has_annotation<graphics::uniform>(mem)) {

                    if (std::meta::is_same_type(std::meta::type_of(mem), ^^texture) || std::meta::is_same_type(std::meta::type_of(mem), ^^buffer_view)) {
                        std::meta::info substitute_result = std::meta::substitute(^^binding_wrapper, {
                                                                                                         std::meta::type_of(mem), std::meta::reflect_constant(meta::get_annotation<graphics::uniform>(mem).value().binding) });
                        result.push_back(std::meta::data_member_spec(substitute_result, { .name = std::meta::identifier_of(mem) }));
                    } else {
                        std::meta::info substitute_result = std::meta::substitute(^^binding_wrapper, {
                                                                                                         ^^buffer, std::meta::reflect_constant(meta::get_annotation<graphics::uniform>(mem).value().binding) });
                        result.push_back(std::meta::data_member_spec(substitute_result, { .name = std::meta::identifier_of(mem) }));
                    }
                }
            }

            std::meta::define_aggregate(^^data_type, result);
        }

        descriptor_factory(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline) : m_descriptor(_descriptor), m_device(_device), m_pipeline(_pipeline) {}

        descriptor_set allocate_set(const std::vector<data_type>& _data) const {
            constexpr auto ctx = std::meta::access_context::current();

            std::vector<mars::descriptor_set_create_params> set_params;

            for (const data_type& data : _data) {
                mars::descriptor_set_create_params params;
                template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^data_type, ctx))) {
                    using C = typename[:std::meta::type_of(mem):];
                    if constexpr (std::is_same_v<typename C::type, texture>)
                        params.textures.emplace_back(data.[:mem:].value, C::binding);
                    else if constexpr (std::is_same_v<typename C::type, buffer_view>)
                        params.buffer_views.emplace_back(data.[:mem:].value, C::binding);
                    else if constexpr (std::is_same_v<typename C::type, buffer>)
                        params.buffers.emplace_back(data.[:mem:].value, C::binding);
                    else
                        throw "this is not supposed to hit, check if you added support to a new descriptor type and forgot to support it here";
                }
                set_params.push_back(std::move(params));
            }

            return mars::graphics::descriptor_set_create(m_descriptor, m_device, m_pipeline, set_params);
        }
    };

    template <typename T>
    class mesh_data {
      private:
        std::vector<buffer> m_buffers;
        buffer m_indice_buffer;
        device m_device;

      public:
        mesh_data() = default;
        mesh_data(const device& _device, const std::vector<buffer>& _buffers, const buffer& _indice) : m_device(_device), m_buffers(_buffers), m_indice_buffer(_indice) {}

        template <auto Member>
            requires(std::is_same_v<typename meta::member_pointer_info<decltype(Member)>::parent, T>)
        void stage(staging_buffer& _stanging, meta::member_pointer_info<decltype(Member)>::type* _ptr) {
            constexpr std::meta::info info = meta::get_member_variable_by_index(^^T, meta::get_member_position<Member>());

            static_assert(meta::has_annotation<graphics::input>(info), "member variable is not a input");

            size_t index = 0;

            constexpr std::meta::access_context ctx = std::meta::access_context::current();

            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                using C = typename[:std::meta::type_of(mem):];

                if constexpr (info == mem)
                    break;

                if constexpr (meta::has_annotation<graphics::input>(mem))
                    ++index;
            }

            mars::graphics::staging_buffer_copy(_stanging, _ptr, m_buffers[index]);
        }

        void stage_index(staging_buffer& _staging, uint16_t* _indices) {
            mars::graphics::staging_buffer_copy(_staging, _indices, m_indice_buffer);
        }

        void bind(const command_buffer& _command_buffer) {
            for (buffer& entry : m_buffers)
                mars::graphics::buffer_bind(entry, _command_buffer);
            if (m_indice_buffer.engine)
                mars::graphics::buffer_bind_index(m_indice_buffer, _command_buffer);
        }

        void draw(command_buffer& _command_buffer) {
            mars::graphics::command_buffer_draw_indexed(_command_buffer, { m_indice_buffer.allocated_size / sizeof(uint16_t), 1, 0, 0, 0 });
        }

        void destroy() {
            for (buffer& entry : m_buffers)
                mars::graphics::buffer_destroy(entry, m_device);
            if (m_indice_buffer.engine)
                mars::graphics::buffer_destroy(m_indice_buffer, m_device);
            m_buffers.clear();
            m_indice_buffer = {};
        }
    };

    template <typename T>
    class pipeline_factory {
      private:
        pipeline m_pipeline;
        device m_device;
        mars::descriptor_create_params descriptor_pool_params;

      public:
        pipeline_factory(const device& _device, const shader& _shader, const render_pass& _render_pass) : m_device(_device) {
            graphics::pipeline_setup_builder setup;
            setup.set_shader(_shader);

            constexpr std::meta::access_context ctx = std::meta::access_context::current();

            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                using C = typename[:std::meta::type_of(mem):];
                if constexpr (meta::has_annotation<graphics::input>(mem))
                    setup.add_input_description<C>(meta::get_annotation<graphics::input>(mem).value().binding);
                else if constexpr (meta::has_annotation<graphics::uniform>(mem)) {
                    pipeline_descriptior_layout layout;
                    if constexpr (std::is_same_v<C, texture>) {
                        layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_IMAGE_SAMPLER;
                        ++descriptor_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
                    } else if constexpr (std::is_same_v<C, buffer_view>) {
                        layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                        ++descriptor_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER];
                    } else {
                        layout.descriptor_type = MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        ++descriptor_pool_params.descriptors_size[MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER];
                    }

                    layout.stage = meta::get_annotation<graphics::uniform>(mem).value().stage;
                    layout.binding = meta::get_annotation<graphics::uniform>(mem).value().binding;
                    setup.add_descriptor(layout);
                }
            }

            m_pipeline = graphics::pipeline_create(_device, _render_pass, setup.build());
        }

        const pipeline& get_pipeline() const { return m_pipeline; }

        descriptor_factory<T> create_descriptor(size_t _frames_in_flight, size_t _pool_size) {
            mars::descriptor_create_params params = descriptor_pool_params;
            for (size_t& entry : params.descriptors_size)
                entry = entry * _frames_in_flight * _pool_size;
            params.max_sets = _pool_size;
            mars::descriptor descriptor = graphics::descriptor_create(m_device, params, _frames_in_flight);
            return descriptor_factory<T>(descriptor, m_device, m_pipeline);
        }

        mesh_data<T> create_input_buffers(size_t _entries, bool _enable_indices) {
            constexpr std::meta::access_context ctx = std::meta::access_context::current();

            std::vector<buffer> result;

            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                if constexpr (meta::has_annotation<graphics::input>(mem)) {
                    buffer new_buffer = mars::graphics::buffer_create(m_device, { .buffer_type = MARS_BUFFER_TYPE_VERTEX | MARS_BUFFER_TYPE_TRANSFER_DST, .buffer_property = MARS_BUFFER_PROPERTY_DEVICE_LOCAL, .allocated_size = _entries * std::meta::size_of(mem) });
                    result.push_back(new_buffer);
                }
            }

            buffer indice_buffer;

            if (_enable_indices)
                indice_buffer = mars::graphics::buffer_create(m_device, { .buffer_type = MARS_BUFFER_TYPE_INDEX | MARS_BUFFER_TYPE_TRANSFER_DST, .buffer_property = MARS_BUFFER_PROPERTY_DEVICE_LOCAL, .allocated_size = _entries * sizeof(uint16_t) });

            return mesh_data<T>(m_device, result, indice_buffer);
        }
    };

    namespace prop {
        static constexpr graphics::input input(size_t _binding, mars_pipeline_stage _stage = MARS_PIPELINE_STAGE_VERTEX) { return { .binding = _binding, .stage = _stage }; }
        static constexpr graphics::uniform uniform(size_t _binding, mars_pipeline_stage _stage = MARS_PIPELINE_STAGE_VERTEX) { return { .binding = _binding, .stage = _stage }; }
    } // namespace prop

} // namespace mars
#pragma once

#include "mars/graphics/backend/format.hpp"
#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>
#include <mars/graphics/backend/pipeline.hpp>

#include <meta>
#include <vector>

namespace mars {
    namespace graphics {

        struct pipeline_setup_builder {
            pipeline_setup setup;

            pipeline_setup_builder& set_shader(const shader& _shader) {
                setup.pipeline_shader = _shader;
                return *this;
            }

            template <typename T>
            pipeline_setup_builder& add_input_description(size_t _binding = 0) {
                constexpr std::meta::access_context ctx = std::meta::access_context::current();

                constexpr size_t size = std::meta::nonstatic_data_members_of(^^T, ctx).size();

                pipeline_binding_description description;
                description.binding = _binding;
                description.stride = sizeof(T);

                setup.attributes.reserve(setup.attributes.size() + size);

                size_t location = 0;

                template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                    pipeline_attribute_description attribute;
                    attribute.location = location++;
                    attribute.binding = _binding;
                    attribute.offset = std::meta::offset_of(mem).bytes;
                    using C = [:std::meta::type_of(mem):];
                    attribute.input_format = make_format<C>();
                    setup.attributes.push_back(attribute);
                }

                setup.bindings.push_back(description);

                return *this;
            }

            pipeline_setup build() const { return setup; }
        };

        inline pipeline pipeline_create(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup) {
            return _device.engine->get_impl<pipeline_impl>().pipeline_create(_device, _render_pass, _setup);
        }

        inline void pipeline_bind(pipeline& _pipeline, const command_buffer& _command_buffer, const pipeline_bind_params& _params) {
            _pipeline.engine->get_impl<pipeline_impl>().pipeline_bind(_pipeline, _command_buffer, _params);
        }

        inline void pipeline_destroy(pipeline& _pipeline, const device& _device) {
            _pipeline.engine->get_impl<pipeline_impl>().pipeline_destroy(_pipeline, _device);
        }
    } // namespace graphics
} // namespace mars
#pragma once

#include <mars/graphics/backend/shader.hpp>

namespace mars {
    namespace prop {
        struct shader_annotation {
            mars_shader_type type;
        };

        inline static constexpr shader_annotation shader(mars_shader_type _type) { return { _type }; }
    } // namespace prop

    namespace graphics {

        template <typename T>
        inline shader shader_create(device& _device, const T& _shaders) {
            std::vector<shader_module> modules;

            static constexpr auto ctx = std::meta::access_context::current();
            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                constexpr std::optional<prop::shader_annotation> value = mars::meta::get_annotation<prop::shader_annotation>(mem);
                if constexpr (value.has_value()) {
                    modules.push_back({ .type = value.value().type,
                                        .path = _shaders.[:mem:] });
                }
            }

            return _device.engine->get_impl<shader_impl>().shader_create(_device, modules);
        }

        inline void shader_destroy(shader& _shader, const device& _device) {
            _shader.engine->get_impl<shader_impl>().shader_destroy(_shader, _device);
        }
    } // namespace graphics
} // namespace mars
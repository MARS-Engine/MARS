#pragma once

#include "buffer.hpp"
#include "mars/graphics/shader.hpp"
#include "pipeline.hpp"
#include <experimental/meta>
#include <glad/gl.h>
#include <meta>

enum mars_draw_type {
    MARS_DRAW_TYPE_TRIANGLES,
};

enum mars_draw_mode {
    MARS_DRAW_MODE_ELEMENTS,
    MARS_DRAW_MODE_ARRAY,
};

namespace mars {
    namespace graphics {
        constexpr GLenum draw_type_to_gl(mars_draw_type _type) {
            switch (_type) {
            case MARS_DRAW_TYPE_TRIANGLES:
                return GL_TRIANGLES;
            }
        }

        struct draw_prop_annotation {
            mars_draw_type type;
            mars_draw_mode mode;
        };

        template <typename T>
        void prepare_draw(const T& _pipeline) {
            constexpr std::optional<draw_prop_annotation> annoation = mars::meta::get_annotation<draw_prop_annotation>(^^T);

            if constexpr (!annoation.has_value()) {
                static_assert(0, "pipeline draw call missing draw_propery");
            }

            static constexpr auto ctx = std::meta::access_context::current();
            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                if constexpr (std::meta::is_same_type(std::meta::type_of(mem), ^^buffer))
                    mars::graphics::bind_buffer(_pipeline.[:mem:]);
                else if constexpr (std::meta::is_same_type(std::meta::type_of(mem), ^^program))
                    mars::graphics::use_program(_pipeline.[:mem:]);
            }
        }

        template <typename T>
        void draw(const T& _pipeline, size_t _size) {
            constexpr std::optional<draw_prop_annotation> annoation = mars::meta::get_annotation<draw_prop_annotation>(^^T);

            if constexpr (!annoation.has_value())
                static_assert(0, "pipeline draw call missing draw_propery");

            if constexpr (annoation.value().mode == MARS_DRAW_MODE_ELEMENTS)
                glDrawElements(draw_type_to_gl(annoation.value().type), _size, GL_UNSIGNED_INT, 0);
            else
                glDrawArrays(draw_type_to_gl(annoation.value().type), 0, _size);
        }

        template <typename T>
        void draw_instanced(const T& _pipeline, size_t _size, size_t _instance_count) {
            constexpr std::optional<draw_prop_annotation> annoation = mars::meta::get_annotation<draw_prop_annotation>(^^T);

            if constexpr (!annoation.has_value())
                static_assert(0, "pipeline draw call missing draw_propery");

            glDrawElementsInstanced(GL_TRIANGLES, _size, GL_UNSIGNED_INT, 0, _instance_count);
        }
    } // namespace graphics

    namespace prop {
        static consteval graphics::draw_prop_annotation draw(mars_draw_type _type, mars_draw_mode _mode = MARS_DRAW_MODE_ELEMENTS) {
            return { _type, _mode };
        };
    } // namespace prop

} // namespace mars
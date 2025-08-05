#pragma once

#include "mars/math/annotation.hpp"
#include "misc.hpp"
#include <SDL3/SDL_opengl.h>
#include <cstddef>
#include <mars/meta.hpp>
#include <meta>

namespace mars {
    namespace graphics {
        struct uniform_prop_annotation {
            unsigned int location;
        };

        template <typename T>
        void set_uniform(const T& _value) {
            static constexpr auto ctx = std::meta::access_context::current();
            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                constexpr std::optional<uniform_prop_annotation> annotation = mars::meta::get_annotation<uniform_prop_annotation>(mem);
                if constexpr (std::meta::is_class_type(std::meta::type_of(mem)) && annotation.has_value()) {
                    constexpr std::meta::info mem_type = std::meta::nonstatic_data_members_of(std::meta::type_of(mem), ctx)[0];
                    int size = 0;

                    if constexpr (!is_valid_gl_struct_type(mem))
                        static_assert(false, "complex types (vec3/matrix/etc) cant have multiple types simples types (basically you cant have a vector with x bein a float and y a int)");

                    template for (constexpr auto sub_mem : std::define_static_array(std::meta::nonstatic_data_members_of(std::meta::type_of(mem), ctx))) {
                        size++;
                    }

                    constexpr unsigned int location = annotation.value().location;

                    if constexpr (mars::meta::has_annotation<mars::matrix_annotation>(mem)) {
                        glUniformMatrix4fv(location, 1, true, reinterpret_cast<const float*>(&_value.[:mem:]));
                    } else {
                        if (size == 1) {
                            if constexpr (mars::meta::is_type<float>(mem_type))
                                glUniform1fv(location, 1, reinterpret_cast<const float*>(&_value.[:mem:]));
                            else if (mars::meta::is_type<double>(mem_type))
                                glUniform1dv(location, 1, reinterpret_cast<const double*>(&_value.[:mem:]));
                        } else if (size == 3) {
                            if constexpr (mars::meta::is_type<float>(mem_type))
                                glUniform3fv(location, 1, reinterpret_cast<const float*>(&_value.[:mem:]));
                            else if (mars::meta::is_type<double>(mem_type))
                                glUniform3dv(location, 1, reinterpret_cast<const double*>(&_value.[:mem:]));
                        } else if (size == 4) {
                            if constexpr (mars::meta::is_type<float>(mem_type))
                                glUniform4fv(location, 1, reinterpret_cast<const float*>(&_value.[:mem:]));
                            else if (mars::meta::is_type<double>(mem_type))
                                glUniform4dv(location, 1, reinterpret_cast<const double*>(&_value.[:mem:]));
                        }
                    }
                }
            }
        }
    } // namespace graphics

    namespace prop {
        static consteval mars::graphics::uniform_prop_annotation uniform(unsigned int _location) {
            return { _location };
        };
    } // namespace prop
} // namespace mars
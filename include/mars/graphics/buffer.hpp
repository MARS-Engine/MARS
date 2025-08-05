#pragma once

#include "misc.hpp"
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_scancode.h>
#include <cstddef>
#include <experimental/meta>
#include <mars/meta.hpp>
#include <meta>

enum mars_buffer_type {
    MARS_BUFFER_TYPE_VERTEX,
    MARS_BUFFER_TYPE_INDICE
};

enum mars_buffer_map {
    MARS_BUFFER_MAP_STATIC,
    MARS_BUFFER_MAP_DYNAMIC,
};

namespace mars {
    namespace graphics {

        struct buffer_annotation {
            mars_buffer_type type;
            mars_buffer_map map;
            size_t binding_divisor;
            bool dummy;
        };

        struct buffer_entry_annotation {
            size_t location;
        };

        constexpr GLenum buffer_type_to_gl(mars_buffer_type _type) {
            switch (_type) {
            case MARS_BUFFER_TYPE_VERTEX:
                return GL_ARRAY_BUFFER;
            case MARS_BUFFER_TYPE_INDICE:
                return GL_ELEMENT_ARRAY_BUFFER;
            }
        }

        constexpr GLenum buffer_map_to_gl(mars_buffer_map _type) {
            switch (_type) {
            case MARS_BUFFER_MAP_STATIC:
                return 0;
            case MARS_BUFFER_MAP_DYNAMIC:
                return GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
            }
        }

        template <typename T>
        struct buffer_data {
            T* data;
            size_t size;
        };

        struct buffer_entry {
            unsigned int id;
            unsigned int binding_id;
        };

        struct buffer {
            unsigned int buffer_id;
            std::vector<buffer_entry> data_ids;
        };

        template <typename T>
        void make_buffer(const T& _data, buffer& _buffer) {
            glGenVertexArrays(1, &_buffer.buffer_id);
            glBindVertexArray(_buffer.buffer_id);

            int next_allocation_index = 0;

            static constexpr auto ctx = std::meta::access_context::current();
            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                constexpr std::optional<buffer_annotation> annoation = mars::meta::get_annotation<buffer_annotation>(mem);
                if constexpr (annoation.has_value()) {
                    unsigned int buffer_id;
                    size_t size = sizeof(*_data.[:mem:].data);

                    glCreateBuffers(1, &buffer_id);
                    glNamedBufferStorage(buffer_id, _data.[:mem:].size * size, _data.[:mem:].data, buffer_map_to_gl(annoation.value().map));

                    unsigned int curr_index = -1;

                    switch (annoation.value().type) {
                    case MARS_BUFFER_TYPE_VERTEX:
                        curr_index = next_allocation_index++;
                        glVertexArrayVertexBuffer(_buffer.buffer_id, curr_index, buffer_id, 0, size);
                        break;
                    case MARS_BUFFER_TYPE_INDICE:
                        glVertexArrayElementBuffer(_buffer.buffer_id, buffer_id);
                        break;
                    }

                    if constexpr (annoation.value().type == MARS_BUFFER_TYPE_VERTEX) {
                        constexpr auto type_ptr = std::meta::template_arguments_of(std::meta::type_of(mem))[0];
                        size_t stride = 0;
                        template for (constexpr auto sub_mem : std::define_static_array(std::meta::nonstatic_data_members_of(type_ptr, ctx))) {
                            constexpr std::optional<buffer_entry_annotation> entry_annoation = mars::meta::get_annotation<buffer_entry_annotation>(sub_mem);

                            if constexpr (entry_annoation.has_value()) {
                                constexpr size_t location = entry_annoation.value().location;
                                constexpr std::meta::info mem_type = std::meta::nonstatic_data_members_of(std::meta::type_of(sub_mem), ctx)[0];

                                glEnableVertexArrayAttrib(_buffer.buffer_id, location);
                                glVertexArrayAttribFormat(_buffer.buffer_id, location, std::meta::size_of(sub_mem) / std::meta::size_of(mem_type), c_type_to_gl_type(mem_type), GL_FALSE, stride);
                                glVertexArrayAttribBinding(_buffer.buffer_id, location, curr_index);
                                glVertexArrayBindingDivisor(_buffer.buffer_id, curr_index, annoation.value().binding_divisor);
                                stride += std::meta::size_of(sub_mem);
                            }
                        }
                    }

                    _buffer.data_ids.push_back({ buffer_id, curr_index });
                }
            }
        }

        template <auto MemberPtr, typename C = meta::template_arg_t<0, typename meta::member_pointer_info<decltype(MemberPtr)>::type>>
        C* map_buffer(const buffer& _buffer, size_t _size, size_t _offset = 0) {
            return static_cast<C*>(glMapNamedBufferRange(_buffer.data_ids[meta::get_member_position<MemberPtr>()].id, _offset * sizeof(C), _size * sizeof(C), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
        }

        template <auto MemberPtr>
        void unmap_buffer(const buffer& _buffer) {
            glUnmapNamedBuffer(_buffer.data_ids[meta::get_member_position<MemberPtr>()].id);
        }

        void unmap_buffer(const buffer& _buffer) {
            glUnmapNamedBuffer(_buffer.buffer_id);
        }

        void bind_buffer(const buffer& _buffer) {
            glBindVertexArray(_buffer.buffer_id);
        }

        void unbind_buffer(const buffer& _buffer) {
            glBindVertexArray(0);
        }

        void destroy_buffers(buffer& _buffer) {
            for (auto& entry : _buffer.data_ids)
                glDeleteBuffers(1, &entry.id);

            glDeleteVertexArrays(1, &_buffer.buffer_id);
            _buffer = {};
        }
    } // namespace graphics

    namespace prop {
        static consteval mars::graphics::buffer_annotation buffer(mars_buffer_type _type, mars_buffer_map _map = MARS_BUFFER_MAP_STATIC, size_t _binding_advisor = 0, bool _dummy = false) {
            return { _type, _map, _binding_advisor, _dummy };
        };

        static consteval mars::graphics::buffer_annotation buffer_dynamic(size_t _binding_advisor = 0) {
            return buffer(MARS_BUFFER_TYPE_VERTEX, MARS_BUFFER_MAP_DYNAMIC, _binding_advisor, true);
        };

        static consteval mars::graphics::buffer_entry_annotation buffer_entry(size_t _location) {
            return { _location };
        };
    } // namespace prop
} // namespace mars
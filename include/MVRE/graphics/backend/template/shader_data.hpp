#ifndef __MVRE__SHADER__DATA__
#define __MVRE__SHADER__DATA__

#include "graphics_component.hpp"
#include "buffer.hpp"
#include "shader.hpp"

namespace mvre_graphics {

    class uniform {
    protected:
        buffer* m_buffer = nullptr;
        shader* m_shader = nullptr;
        size_t m_index = 0;
    public:
        explicit uniform(size_t _index, shader* _shader, buffer* _buffer) { m_index = _index; m_shader = _shader; m_buffer = _buffer; }

        inline void update(void* _data) { m_buffer->bind(); m_buffer->copy_data(_data); m_buffer->unbind(); }
        virtual void bind() { }
    };

    class shader_data : public graphics_component {
    protected:
        std::map<std::string, uniform*> m_uniforms;
    public:
        using graphics_component::graphics_component;

        virtual void generate(shader* _shader) { }

        virtual void bind() { }

        uniform* get_uniform(const std::string& _uniform) { return m_uniforms[_uniform]; }
    };
}

#endif
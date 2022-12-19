#ifndef MVRE_SHADER_DATA_
#define MVRE_SHADER_DATA_

#include "graphics_component.hpp"
#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace mvre_graphics {

    class uniform {
    protected:
        std::vector<buffer*> m_buffers;
        shader* m_shader = nullptr;
        size_t m_index = 0;
        mvre_shader_uniform* m_uni_data;
    public:
        mvre_shader_uniform* get_data() { return m_uni_data; }

        explicit uniform(mvre_shader_uniform* _uni_data, size_t _index, shader* _shader, std::vector<buffer*>& _buffers) { m_uni_data = _uni_data; m_index = _index; m_shader = _shader; m_buffers = _buffers; }
        explicit uniform(mvre_shader_uniform* _uni_data, size_t _index, shader* _shader) { m_uni_data = _uni_data; m_index = _index; m_shader = _shader; }

        inline void update(size_t _frame, void* _data) { m_buffers[_frame]->copy_data(_data); }
        virtual void bind(size_t _index) { }
        virtual void destroy() { }
    };

    class shader_data : public graphics_component {
    protected:
        std::map<std::string, uniform*> m_uniforms;
        std::map<std::string, texture*> m_textures;
        shader* m_shader = nullptr;
        pipeline* m_pipeline;

    public:
        using graphics_component::graphics_component;

        virtual void generate(pipeline* _pipeline, shader* _shader) { }

        void set_textures(const std::map<std::string, texture*>& _textures) {
            m_textures = _textures;
        }

        virtual void bind() { }
        virtual void destroy() { }

        void update(const std::string& _uniform, void* _data) { m_uniforms[_uniform]->update(instance()->current_frame(), _data); }

        uniform* get_uniform(const std::string& _uniform) { return m_uniforms[_uniform]; }
    };
}

#endif
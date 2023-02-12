#ifndef MARS_SHADER_DATA_
#define MARS_SHADER_DATA_

#include "graphics_component.hpp"
#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace mars_graphics {

    class uniform {
    protected:
        buffer* m_buffer = nullptr;
        shader* m_shader = nullptr;
        size_t m_index = 0;
        mars_shader_uniform* m_uni_data = nullptr;
    public:
        mars_shader_uniform* get_data() { return m_uni_data; }

        [[nodiscard]] size_t get_buffer_size() const { return m_buffer->size(); }

        explicit uniform(mars_shader_uniform* _uni_data, size_t _index, shader* _shader, buffer* _buffer = nullptr) { m_uni_data = _uni_data; m_index = _index; m_shader = _shader; m_buffer = _buffer; }

        inline void copy_offset(size_t _index, size_t _offset, size_t _size, void* _data) { m_buffer->copy_offset(_offset, _size, _data); }

        inline void copy_offset(size_t _offset, size_t _size, void* _data) {
            m_buffer->copy_offset(_offset, _size, _data);
        }

        inline void copy_data(size_t _index) { m_buffer->copy_data(_index); }

        inline void update(void* _data) {
            m_buffer->update(_data);
        }

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

        std::map<std::string, uniform*>& get_uniforms() { return m_uniforms; }
        std::map<std::string, texture*>& get_textures() { return m_textures; }

        virtual void generate(pipeline* _pipeline, shader* _shader) { }

        void set_textures(const std::map<std::string, texture*>& _textures) {
            m_textures = _textures;
        }

        virtual void bind_textures() { }
        virtual void bind() { }
        virtual void destroy() { }

        void update(const std::string& _uniform, void* _data) { m_uniforms[_uniform]->update(_data); }

        uniform* get_uniform(const std::string& _uniform) { return m_uniforms[_uniform]; }
    };
}

#endif
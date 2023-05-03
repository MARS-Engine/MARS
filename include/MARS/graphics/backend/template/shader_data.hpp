#ifndef MARS_SHADER_DATA_
#define MARS_SHADER_DATA_

#include "graphics_component.hpp"
#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "./builders/shader_data_builder.hpp"

namespace mars_graphics {

    class uniform {
    protected:
        std::shared_ptr<buffer> m_buffer;
        mars_ref<shader> m_shader;
        size_t m_index = 0;
        std::shared_ptr<mars_shader_uniform> m_uni_data;
    public:
        std::shared_ptr<mars_shader_uniform> get_data() { return m_uni_data; }

        [[nodiscard]] size_t get_buffer_size() const { return m_buffer->size(); }

        explicit uniform(const std::shared_ptr<mars_shader_uniform>& _uni_data, size_t _index, const mars_ref<shader>& _shader, const std::shared_ptr<buffer>& _buffer = nullptr) { m_uni_data = _uni_data; m_index = _index; m_shader = _shader; m_buffer = _buffer; }

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
        std::map<std::string, std::shared_ptr<uniform>> m_uniforms;
        shader_data_core m_data;

        virtual void generate(const mars_ref<pipeline>& _pipeline, const mars_ref<shader>& _shader) { }

        inline void set_data(const shader_data_core& _data) { m_data = _data; }

        friend shader_data_builder;
    public:
        using graphics_component::graphics_component;

        ~shader_data() {
            m_data.m_textures.clear();
        }

        std::map<std::string, std::shared_ptr<uniform>>& get_uniforms() { return m_uniforms; }
        std::map<std::string, std::shared_ptr<texture>>& get_textures() { return m_data.m_textures; }

        virtual void bind_textures() { }
        virtual void bind() { }

        void update(const std::string& _uniform, void* _data) { m_uniforms[_uniform]->update(_data); }

        std::shared_ptr<uniform> get_uniform(const std::string& _uniform) { return m_uniforms[_uniform]; }
    };
}

#endif
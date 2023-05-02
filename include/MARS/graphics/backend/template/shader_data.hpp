#ifndef MARS_SHADER_DATA_
#define MARS_SHADER_DATA_

#include "graphics_component.hpp"
#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace mars_graphics {

    class uniform {
    protected:
        mars_ref<buffer> m_buffer;
        mars_ref<shader> m_shader;
        size_t m_index = 0;
        std::shared_ptr<mars_shader_uniform> m_uni_data;
    public:
        std::shared_ptr<mars_shader_uniform> get_data() { return m_uni_data; }

        [[nodiscard]] size_t get_buffer_size() const { return m_buffer->size(); }

        explicit uniform(const std::shared_ptr<mars_shader_uniform>& _uni_data, size_t _index, const mars_ref<shader>& _shader, const mars_ref<buffer>& _buffer = mars_ref<buffer>()) { m_uni_data = _uni_data; m_index = _index; m_shader = _shader; m_buffer = _buffer; }

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
        std::map<std::string, std::shared_ptr<texture>> m_textures;
        mars_ref<shader> m_shader;
        mars_ref<pipeline> m_pipeline;
        std::shared_ptr<shader_data> m_next = nullptr;
    public:
        inline std::shared_ptr<shader_data> next() const { return m_next; }
        inline void set_next(std::shared_ptr<shader_data> _next) { m_next = _next; }

        using graphics_component::graphics_component;

        std::map<std::string, std::shared_ptr<uniform>>& get_uniforms() { return m_uniforms; }
        std::map<std::string, std::shared_ptr<texture>>& get_textures() { return m_textures; }

        virtual void generate(const mars_ref<pipeline>& _pipeline, const mars_ref<shader>& _shader) { }

        void set_textures(const std::map<std::string, std::shared_ptr<texture>>& _textures) {
            m_textures = _textures;
        }

        virtual void bind_textures() { }
        virtual void bind() { }
        virtual void destroy() { }

        void update(const std::string& _uniform, void* _data) { m_uniforms[_uniform]->update(_data); }

        std::shared_ptr<uniform> get_uniform(const std::string& _uniform) { return m_uniforms[_uniform]; }
    };
}

#endif
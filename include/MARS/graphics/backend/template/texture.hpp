#ifndef MARS_TEXTURE_
#define MARS_TEXTURE_

#include "graphics_builder.hpp"
#include "graphics_component.hpp"
#include "builders/texture_builder.hpp"
#include <MARS/resources/resource_manager.hpp>


namespace mars_graphics {

    class texture : public graphics_component {
    protected:
        texture_data m_data;
    public:
        using graphics_component::graphics_component;

        [[nodiscard]] inline MARS_FORMAT format() const { return m_data.format; }
        [[nodiscard]] inline MARS_TEXTURE_LAYOUT layout() const { return m_data.layout; }
        [[nodiscard]] mars_math::vector2<size_t> size() const { return m_data.size; }

        inline void set_data(const texture_data& _data) { m_data = _data; }

        virtual void copy_buffer_to_image(buffer* _buffer, const mars_math::vector4<uint32_t>& _rect) { }
        virtual void load_from_file(const std::string& _path) { }

        virtual void initialize() { }
        virtual void complete() { }

        virtual void bind() { }
        virtual void unbind() { }
    };
}

#endif
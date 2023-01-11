#ifndef MARS_FRAMEBUFFER_
#define MARS_FRAMEBUFFER_

#include "graphics_component.hpp"
#include "graphics_types.hpp"
namespace mars_graphics {
    class framebuffer : public graphics_component {
    public:
        using graphics_component::graphics_component;

        virtual void add_texture(MARS_TEXTURE_TYPE _type, MARS_TEXTURE_DATA_TYPE _data_type) { }

        virtual void create() { }

        virtual void bind() { }
        virtual void bind_textures() { }
        virtual void unbind() { }
    };
}

#endif
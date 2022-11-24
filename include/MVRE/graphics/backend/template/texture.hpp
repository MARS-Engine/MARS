#ifndef __MVRE__TEXTURE__
#define __MVRE__TEXTURE__

#include "graphics_component.hpp"
#include <MVRE/resources/resource_manager.hpp>

namespace mvre_graphics {

    class texture : public mvre_resources::resource_base, graphics_component {
    protected:
        mvre_math::vector2<int> m_size {};
        int m_channels = -1;
        unsigned char* m_data = nullptr;

        /**
         * Load texture to m_data
         * @param _texture_path texture path
         */
        bool load_texture(const std::string& _texture_path);

        /**
         * Free original texture. (Execute after loading into OpenGL, Vulkan, etc...
         */
        void free_og_texture();
    public:
        using graphics_component::graphics_component;

        virtual void bind() { }
        virtual void unbind() { }
    };
}

#endif
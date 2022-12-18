#ifndef __MVRE__TEXTURE__
#define __MVRE__TEXTURE__

#include "graphics_component.hpp"
#include <MVRE/resources/resource_manager.hpp>

namespace mvre_graphics {

    class texture : public mvre_resources::resource_base, public graphics_component {
    protected:
        size_t m_index = 0;
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

        inline void set_index(size_t _index) { m_index  = _index; }

        virtual void bind() { }
        virtual void unbind() { }
    };
}

#endif
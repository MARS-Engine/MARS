#ifndef MARS_TEXTURE_
#define MARS_TEXTURE_

#include "graphics_component.hpp"
#include <MARS/resources/resource_manager.hpp>

namespace mars_graphics {

    class texture : public mars_resources::resource_base, public graphics_component {
    protected:
        size_t m_index = 0;
        mars_math::vector2<int> m_size {};
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

        inline size_t get_index() { return m_index; }
        inline void set_index(size_t _index) { m_index  = _index; }

        virtual void bind() { }
        virtual void unbind() { }
    };
}

#endif
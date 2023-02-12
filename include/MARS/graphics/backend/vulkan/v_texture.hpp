#ifndef MARS_V_TEXTURE_
#define MARS_V_TEXTURE_

#include <MARS/graphics/backend/template/texture.hpp>
#include <vulkan/vulkan.h>

namespace mars_graphics {
    class v_buffer;
    class v_image;

    class v_texture : public texture {
    private:
        v_image* m_image = nullptr;
        VkSampler m_sampler = nullptr;

        void copy_buffer_to_image(v_buffer& buffer, VkImage _image);
        void transition_image_layout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void create_sampler();
    public:

        VkImageView raw_image_view();
        inline VkSampler raw_sampler() { return m_sampler; }

        using texture::texture;

        bool load_resource(const std::string &_texture_path) override;
        void create(MARS_FORMAT _format, MARS_TEXTURE_USAGE _usage) override;
        void clean() override;
    };
}

#endif
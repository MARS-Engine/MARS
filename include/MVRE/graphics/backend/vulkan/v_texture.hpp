#ifndef __MVRE__V__TEXTURE__
#define __MVRE__V__TEXTURE__

#include <MVRE/graphics/backend/template/texture.hpp>
#include <vulkan/vulkan.h>

namespace mvre_graphics {
    class v_buffer;

    class v_texture : public texture {
    private:

        VkImage m_image;
        VkImageView m_image_view;
        VkDeviceMemory m_image_memory;
        VkSampler m_sampler;
        VkFormat m_format;

        void copy_buffer_to_image(v_buffer& buffer, VkImage _image);
        void transition_image_layout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void create_image();
        void create_image_view(VkFormat _format);
        void create_sampler();
    public:
        using texture::texture;

        bool load_resource(const std::string &_texture_path) override;
        void clean() override;
    };
}

#endif
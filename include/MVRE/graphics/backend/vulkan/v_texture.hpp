#ifndef MVRE_V_TEXTURE_
#define MVRE_V_TEXTURE_

#include <MVRE/graphics/backend/template/texture.hpp>
#include <vulkan/vulkan.h>

namespace mvre_graphics {
    class v_buffer;
    class v_image;

    class v_texture : public texture {
    private:
        v_image* m_image;
        VkSampler m_sampler;

        void copy_buffer_to_image(v_buffer& buffer, VkImage _image);
        void transition_image_layout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void create_sampler();
    public:

        VkImageView raw_image_view();
        inline VkSampler raw_sampler() { return m_sampler; }

        using texture::texture;

        bool load_resource(const std::string &_texture_path) override;
        void clean() override;
    };
}

#endif
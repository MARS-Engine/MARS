#ifndef MARS_V_TEXTURE_
#define MARS_V_TEXTURE_

#include <MARS/graphics/backend/template/texture.hpp>
#include <vulkan/vulkan.h>

namespace mars_graphics {
    class v_buffer;
    class v_image;

    class v_texture : public texture {
    private:
        bool transition_enabled = false;
        v_image* m_image = nullptr;
        VkSampler m_sampler = nullptr;

        void copy_buffer_to_image(v_buffer* buffer, const mars_math::vector4<uint32_t>& _rect);
        void transition_image_layout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void create_sampler();
    public:
        using texture::texture;

        ~v_texture();

        VkImageView raw_image_view();
        inline VkSampler raw_sampler() { return m_sampler; }

        void load_from_file(const std::string &_path) override;
        void copy_buffer_to_image(const std::shared_ptr<buffer>& _buffer, const mars_math::vector4<uint32_t> &_rect) override;

        void initialize() override;
        void complete() override;
    };
}

#endif
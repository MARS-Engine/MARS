#ifndef MARS_V_IMAGE_
#define MARS_V_IMAGE_

#include "v_base.hpp"

namespace mars_graphics  {

    class v_image : public v_base {
    private:
        VkImage m_image;
        VkImageView m_image_view;
        VkDeviceMemory m_image_memory;
        VkFormat m_format;
        VkImageUsageFlags m_usage;
        VkImageAspectFlags m_aspect_flag = VK_IMAGE_ASPECT_COLOR_BIT;

        mars_math::vector2<uint32_t> m_size;
    public:
        using v_base::v_base;

        [[nodiscard]] VkImage raw_image() const { return m_image; };
        [[nodiscard]] VkImageView raw_image_view() const { return m_image_view; };

        void set_usage(VkImageUsageFlags _usage) { m_usage = _usage | VK_IMAGE_USAGE_SAMPLED_BIT; }

        VkFormat format() { return m_format; };
        void set_format(VkFormat _format) { m_format = _format; }

        void set_size(const mars_math::vector2<uint32_t>& _size) { m_size = _size; }
        void set_size(mars_math::vector2<size_t> _size) { m_size = { static_cast<uint32_t>(_size.x()), static_cast<uint32_t>(_size.y()) }; }

        void create_image(VkImageAspectFlags _aspect_flag);
        void create_image_view();

        void destroy();
    };
}

#endif
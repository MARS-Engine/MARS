#ifndef __MVRE__V__IMAGE__
#define __MVRE__V__IMAGE__

#include "v_base.hpp"

namespace mvre_graphics  {

    class v_image : public v_base {
    private:
        VkImage m_image;
        VkImageView m_image_view;
        VkDeviceMemory m_image_memory;
        VkFormat m_format;
        VkImageUsageFlags m_usage;
        VkImageAspectFlags m_aspect_flag = VK_IMAGE_ASPECT_COLOR_BIT;

        mvre_math::vector2<uint32_t> m_size;
    public:
        using v_base::v_base;

        VkImage raw_image() { return m_image; };
        VkImageView raw_image_view() { return m_image_view; };

        void set_usage(VkImageUsageFlags _usage) { m_usage = _usage; }
        void set_aspect_flag(VkImageAspectFlags _aspect_flag) { m_aspect_flag = _aspect_flag; }

        VkFormat format() { return m_format; };
        void set_format(VkFormat _format) { m_format = _format; }

        void set_size(const mvre_math::vector2<uint32_t>& _size) { m_size = _size; }
        void set_size(mvre_math::vector2<int> _size) { m_size = { static_cast<uint32_t>(_size.x()), static_cast<uint32_t>(_size.y()) }; }

        void create_image();
        void create_image_view();

        void destroy();
    };
}

#endif
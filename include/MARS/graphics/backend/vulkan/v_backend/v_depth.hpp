#ifndef MARS_V_DEPTH_
#define MARS_V_DEPTH_

#include "v_base.hpp"
#include "v_image.hpp"

namespace mars_graphics {

    class v_depth : public v_base {
    private:
        v_image* image = nullptr;
         MARS_FORMAT m_format;

        VkFormat find_supported_format(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features);
    public:
        VkFormat find_depth_format();
        inline VkImageView get_image_view() { return image->raw_image_view(); }
        inline MARS_FORMAT get_format() { return m_format; }

        using v_base::v_base;

        void create();
        void destroy();
    };
}

#endif
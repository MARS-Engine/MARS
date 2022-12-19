#ifndef __MVRE__V__DEPTH__
#define __MVRE__V__DEPTH__

#include "v_base.hpp"
#include "v_image.hpp"

namespace mvre_graphics {

    class v_depth : public v_base {
    private:
        v_image* image;

        VkFormat find_supported_format(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features);
    public:
        VkFormat find_depth_format();
        inline VkImageView get_image_view() { return image->raw_image_view(); }

        using v_base::v_base;

        void create();
        void destroy();
    };
}

#endif
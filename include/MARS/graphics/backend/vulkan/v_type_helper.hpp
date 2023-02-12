#ifndef MARS_V_TYPE_HELPER_
#define MARS_V_TYPE_HELPER_

#include <MARS/graphics/backend/template/graphics_types.hpp>
#include <vulkan/vulkan.h>

namespace mars_graphics {

    VkFormat MARS2VK(MARS_FORMAT _format);
    VkImageUsageFlagBits MARS2VK(MARS_TEXTURE_USAGE _usage);
    VkImageLayout MARS2VK(MARS_TEXTURE_LAYOUT _usage);
    VkPrimitiveTopology MARS2VK(MARS_TOPOLOGY _topology);

    MARS_FORMAT VK2MARS(VkFormat _format);
    MARS_TEXTURE_LAYOUT VK2MARS(VkImageLayout _layout);
}

#endif
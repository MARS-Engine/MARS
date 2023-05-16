#include <MARS/graphics/backend/vulkan/v_backend/v_depth.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>
#include <MARS/graphics/backend/vulkan/v_type_helper.hpp>

using namespace mars_graphics;

VkFormat v_depth::find_supported_format(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features) {
    for (VkFormat format : _candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(graphics()->get_device()->raw_physical_device(), format, &props);

        if ((_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & _features) == _features) || (_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & _features) == _features))
            return format;
    }

    mars_debug::debug::error("MARS - Vulkan - Depth - Failed to find supported format");
}

VkFormat v_depth::find_depth_format() {
    return find_supported_format(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}


void v_depth::create() {
    image = new v_image(graphics());
    image->set_format(find_depth_format());
    image->set_size(graphics()->get_window()->size());
    image->set_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    image->create_image(VK_IMAGE_ASPECT_DEPTH_BIT);
    image->create_image_view();

    m_format = VK2MARS(find_depth_format());
}

void v_depth::destroy() {
    image->destroy();
    delete image;
}
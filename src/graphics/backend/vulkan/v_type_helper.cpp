#include <MARS/graphics/backend/vulkan/v_type_helper.hpp>

VkFormat mars_graphics::MARS2VK(MARS_FORMAT _format) {
    switch (_format) {
        case MARS_FORMAT_UNDEFINED:
            return VK_FORMAT_UNDEFINED;
        case MARS_FORMAT_S_R8:
            return VK_FORMAT_R8_SRGB;
        case MARS_FORMAT_S_RGB8:
            return VK_FORMAT_R8G8B8_SRGB;
        case MARS_FORMAT_S_RGBA8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case MARS_FORMAT_SBGRA_8:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case MARS_FORMAT_F_RGB16:
            return VK_FORMAT_R16G16B16_SFLOAT;
        case MARS_FORMAT_F_RGBA16:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case MARS_FORMAT_DEPTH_32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case MARS_FORMAT_DEPTH_32_SFLOAT_STENCIL_8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case MARS_FORMAT_DEPTH_24_UNORM_STENCIL_8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        default:
            return VK_FORMAT_UNDEFINED;
    }
}

VkImageUsageFlagBits mars_graphics::MARS2VK(MARS_TEXTURE_USAGE _usage) {
    switch (_usage) {
        case MARS_TEXTURE_USAGE_COLOR:
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        case MARS_TEXTURE_USAGE_DEPTH:
            return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case MARS_TEXTURE_USAGE_TRANSFER:
            return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        default:
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
}

VkImageLayout mars_graphics::MARS2VK(MARS_TEXTURE_LAYOUT _layout) {
    switch (_layout) {
        case MARS_TEXTURE_LAYOUT_PRESENT:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case MARS_TEXTURE_LAYOUT_READONLY:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case MARS_TEXTURE_LAYOUT_COLOR_ATTACHMENT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

VkPrimitiveTopology mars_graphics::MARS2VK(MARS_TOPOLOGY _topology) {
    switch (_topology) {
        case MARS_TOPOLOGY_TRIANGLE_LIST:
        default:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case MARS_TOPOLOGY_TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    }
}

mars_graphics::MARS_FORMAT mars_graphics::VK2MARS(VkFormat _format) {
    switch (_format) {
        case VK_FORMAT_R8G8B8_SRGB:
            return MARS_FORMAT_S_RGB8;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return MARS_FORMAT_S_RGBA8;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return MARS_FORMAT_SBGRA_8;
        case VK_FORMAT_R16G16B16_SFLOAT:
            return MARS_FORMAT_F_RGB16;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return MARS_FORMAT_F_RGBA16;
        case VK_FORMAT_D32_SFLOAT:
            return MARS_FORMAT_DEPTH_32_SFLOAT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return MARS_FORMAT_DEPTH_32_SFLOAT_STENCIL_8_UINT;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return MARS_FORMAT_DEPTH_24_UNORM_STENCIL_8_UINT;
        default:
            return MARS_FORMAT::MARS_FORMAT_UNDEFINED;
    }
}

mars_graphics::MARS_TEXTURE_LAYOUT mars_graphics::VK2MARS(VkImageLayout _layout) {
    switch (_layout) {
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return MARS_TEXTURE_LAYOUT_PRESENT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return MARS_TEXTURE_LAYOUT_READONLY;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return MARS_TEXTURE_LAYOUT_COLOR_ATTACHMENT;
        default:
            return MARS_TEXTURE_LAYOUT_UNDEFINED;
    }
}

#include "vtexture.hpp"
#include "vbuffer.hpp"
#include "vdevice.hpp"
#include "vcommand_buffer.hpp"
#include "vpipeline.hpp"
#include "vshader.hpp"
#include "vdescriptor_pool.hpp"
#include "Debug/debug.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

vtexture::vtexture(vdevice* _device, VmaAllocator& _allocator) {
    device = _device;
    allocator = _allocator;
}

void vtexture::generate_image_view(texture_image_view data) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;

    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.image = buffer->image;
    viewInfo.format = data.format;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.aspectMask = data.aspect_mask;

    vkCreateImageView(device->raw_device, &viewInfo, nullptr, &image_view);
}

void vtexture::generate_sampler() {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    vkCreateSampler(device->raw_device, &samplerInfo, nullptr, &sampler);
}

void vtexture::load_texture(const std::string& _texture_location) {
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load(_texture_location.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    size = vector2(texWidth, texHeight);

    VkDeviceSize bufferSize = texWidth * texHeight * 4;
    format = VK_FORMAT_R8G8B8A8_SRGB;

    if (!pixels)
        debug::error("Failed to load vTexture file - " + _texture_location);

    buffer = new vbuffer(device, allocator);
    buffer->Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    buffer->Update(pixels);
    stbi_image_free(pixels);

    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(texWidth);
    imageExtent.height = static_cast<uint32_t>(texHeight);
    imageExtent.depth = 1;

    buffer->CreateImage(format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

    buffer->TransitionImageLayout(true);
    buffer->CopyBufferImage();
    buffer->TransitionImageLayout(false);

    generate_image_view({});
    generate_sampler();
}

void vtexture::create(vector2 _size, VkFormat _format, VkImageUsageFlagBits _usage) {
    size = _size;
    format = _format;

    VkImageAspectFlags aspectMask = 0;

    if (_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    if (_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (format >= VK_FORMAT_D16_UNORM_S8_UINT)
            aspectMask |=VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(size.x);
    imageExtent.height = static_cast<uint32_t>(size.y);
    imageExtent.depth = 1;

    buffer = new vbuffer(device, allocator);
    buffer->CreateImage(format, _usage | VK_IMAGE_USAGE_SAMPLED_BIT, imageExtent);

    generate_image_view({.format = format, .aspect_mask = aspectMask});
    generate_sampler();
}

void vtexture::bind(vcommand_buffer* _command_buffer, vpipeline* _pipeline) const {
    vkCmdBindDescriptorSets(_command_buffer->raw_command_buffers[_command_buffer->record_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->pipeline_layout, 0, 1, &buffer->descriptor, 0, nullptr);
}
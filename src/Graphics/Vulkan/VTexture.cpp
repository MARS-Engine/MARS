#include "VTexture.hpp"
#include "VBuffer.hpp"
#include "VDevice.hpp"
#include "VCommandBuffer.hpp"
#include "VPipeline.hpp"
#include "VShader.hpp"
#include "VDescriptorPool.hpp"
#include "Debug/Debug.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

VTexture::VTexture(VDevice* _device, VmaAllocator& _allocator) {
    device = _device;
    allocator = _allocator;
}

void VTexture::GenerateImageView(TextureImageView data) {
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
    viewInfo.subresourceRange.aspectMask = data.aspectMask;

    vkCreateImageView(device->rawDevice, &viewInfo, nullptr, &imageView);
}

void VTexture::GenerateSampler() {
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

    vkCreateSampler(device->rawDevice, &samplerInfo, nullptr, &textureSampler);
}

void VTexture::LoadTexture(const string& textureLocation) {
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load(textureLocation.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    size = Vector2(texWidth, texHeight);

    VkDeviceSize bufferSize = texWidth * texHeight * 4;
    format = VK_FORMAT_R8G8B8A8_SRGB;

    if (!pixels)
        Debug::Error("Failed to load vTexture file - " + textureLocation);

    buffer = new VBuffer(device, allocator);
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

    GenerateImageView({});
    GenerateSampler();
}

void VTexture::Create(Vector2 _size, VkFormat _format, VkImageUsageFlagBits usage) {
    size = _size;
    format = _format;

    VkImageAspectFlags aspectMask = 0;

    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (format >= VK_FORMAT_D16_UNORM_S8_UINT)
            aspectMask |=VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(size.x);
    imageExtent.height = static_cast<uint32_t>(size.y);
    imageExtent.depth = 1;

    buffer = new VBuffer(device, allocator);
    buffer->CreateImage(format, usage | VK_IMAGE_USAGE_SAMPLED_BIT, imageExtent);

    GenerateImageView({ .format = format, .aspectMask = aspectMask });
    GenerateSampler();
}

void VTexture::Bind(VCommandBuffer* commandBuffer, VPipeline* pipeline) const {
    vkCmdBindDescriptorSets(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0, 1, &buffer->descriptor, 0, nullptr);
}
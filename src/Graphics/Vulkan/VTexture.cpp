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

void VTexture::LoadTexture(string textureLocation) {
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load(textureLocation.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    VkDeviceSize size = texWidth * texHeight * 4;
    VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

    if (!pixels)
        Debug::Error("Failed to load vTexture file - " + textureLocation);

    buffer = new VBuffer(device, allocator);
    buffer->Create(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    buffer->Update(pixels);
    stbi_image_free(pixels);

    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(texWidth);
    imageExtent.height = static_cast<uint32_t>(texHeight);
    imageExtent.depth = 1;

    buffer->CreateImage(Vector2(texWidth, texHeight), image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

    buffer->TransitionImageLayout(true);
    buffer->CopyBufferImage();
    buffer->TransitionImageLayout(false);


    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;

    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.image = buffer->image;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkCreateImageView(device->rawDevice, &viewInfo, nullptr, &imageView);

    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.pNext = nullptr;

    info.magFilter = VK_FILTER_NEAREST;
    info.minFilter = VK_FILTER_NEAREST;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    vkCreateSampler(device->rawDevice, &info, nullptr, &textureSampler);
}

void VTexture::Bind(VCommandBuffer* commandBuffer, VPipeline* pipeline) {
    vkCmdBindDescriptorSets(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0, 1, &buffer->descriptor, 0, nullptr);
}
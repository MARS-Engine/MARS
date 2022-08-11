#include "VShaderData.hpp"
#include "VShader.hpp"
#include "VDevice.hpp"
#include "VDescriptorPool.hpp"
#include "VTexture.hpp"
#include "VBuffer.hpp"
#include "VPipeline.hpp"
#include "VCommandBuffer.hpp"

VUniform::VUniform(VUniformData* _data, VDevice* _device, VmaAllocator& _allocator) {
    data = _data;
    device = _device;
    allocator = _allocator;
    texture = nullptr;
}

void VUniform::Generate(size_t bufferSize, size_t framebufferSize) {
    uniform.resize(framebufferSize, new VBuffer(device, allocator));

    for (auto uni : uniform)
        uni->Create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}

void VUniform::SetTexture(VTexture* _texture) {
    texture = _texture;
}

void VUniform::Update(void *data, size_t index) {
    uniform[index]->Update(data);
}

VShaderData::VShaderData(VShader* _shader, VmaAllocator& _allocator) {
    shader = _shader;
    allocator = _allocator;

    for (auto uni : shader->uniforms)
            uniforms.push_back(new VUniform(uni, shader->device, allocator));
}

VUniform* VShaderData::GetUniform(string name) {
    for (auto uni : uniforms)
        if (uni->data->name == name)
            return  uni;
    Debug::Alert("Failed to find uniform with name " + name);
    return nullptr;
}

void VShaderData::Generate(size_t size) {
    vector<VkDescriptorSetLayoutBinding> LayoutBindings = vector<VkDescriptorSetLayoutBinding>(uniforms.size());

    for (size_t Index = 0; Index < LayoutBindings.size(); Index++)
        LayoutBindings[Index] = uniforms[Index]->data->binding;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(LayoutBindings.size());
    layoutInfo.pBindings = LayoutBindings.data();

    if (vkCreateDescriptorSetLayout(shader->device->rawDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        Debug::Error("failed to create descriptor set layout!");

    descriptorPool = new VDescriptorPool(shader->device);
    descriptorPool->Create(shader);

    std::vector<VkDescriptorSetLayout> layouts(size, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool->rawDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(size);

    VkResult Result = vkAllocateDescriptorSets(shader->device->rawDevice, &allocInfo, descriptorSets.data());

    if (Result != VK_SUCCESS)
        Debug::Error("failed to allocate descriptor sets!");

    for (size_t Index = 0; Index < size; Index++) {
        vector<VkWriteDescriptorSet> descriptorWrites;

        for (size_t SubIndex = 0; SubIndex < uniforms.size(); SubIndex++) {
            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[Index];
            descriptorWrite.dstBinding = SubIndex;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;

            switch (uniforms[SubIndex]->data->type) {
                case UNIFORM_BUFFER: {
                    auto* bufferInfo = new VkDescriptorBufferInfo();
                    bufferInfo->buffer = uniforms[SubIndex]->uniform[Index]->buffer;
                    bufferInfo->offset = 0;
                    bufferInfo->range = uniforms[SubIndex]->uniform[Index]->bufferSize;

                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrite.pBufferInfo = bufferInfo;
                }
                    break;
                case UNIFORM_TEXTURE: {
                    auto* imageInfo = new VkDescriptorImageInfo();
                    imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                    if (uniforms[SubIndex]->texture == nullptr)
                        Debug::Error("Attempted To Genereate Unifrom Buffer Without Setting Texture");

                    imageInfo->imageView = uniforms[SubIndex]->texture->imageView;
                    imageInfo->sampler = uniforms[SubIndex]->texture->textureSampler;

                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.pImageInfo = imageInfo;
                }
                    break;
            }

            descriptorWrites.push_back(descriptorWrite);
        }
        vkUpdateDescriptorSets(shader->device->rawDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void VShaderData::Bind(VCommandBuffer* commandBuffer, VPipeline* pipeline) {
    vkCmdBindDescriptorSets(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0, 1 , &descriptorSets[commandBuffer->recordIndex], 0, nullptr);
}
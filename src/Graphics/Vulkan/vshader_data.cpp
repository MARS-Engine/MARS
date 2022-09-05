#include "vshader_data.hpp"
#include "vshader.hpp"
#include "vdevice.hpp"
#include "vdescriptor_pool.hpp"
#include "vtexture.hpp"
#include "vbuffer.hpp"
#include "vpipeline.hpp"
#include "vcommand_buffer.hpp"

vuniform::vuniform(vuniform_data* _data, vdevice* _device, VmaAllocator& _allocator) {
    data = _data;
    device = _device;
    allocator = _allocator;
    texture = nullptr;
}

void vuniform::generate(size_t _buffer_size, size_t _framebuffer_size) {
    uniform.resize(_framebuffer_size, new vbuffer(device, allocator));

    for (auto uni : uniform)
        uni->Create(_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}

void vuniform::set_texture(vtexture* _texture) {
    texture = _texture;
}

void vuniform::update(void *_data, size_t _index) {
    uniform[_index]->Update(_data);
}

vshader_data::vshader_data(VShader* _shader, VmaAllocator& _allocator) {
    shader = _shader;
    allocator = _allocator;

    for (auto uni : shader->uniforms)
            uniforms.push_back(new vuniform(uni, shader->device, allocator));
}

vuniform* vshader_data::get_uniform(string _name) {
    for (auto uni : uniforms)
        if (uni->data->name == _name)
            return  uni;
    debug::alert("Failed to find uniform with name " + _name);
    return nullptr;
}

void vshader_data::update_descriptors() {
    for (size_t Index = 0; Index < size; Index++) {
        vector<VkWriteDescriptorSet> descriptorWrites;

        for (size_t SubIndex = 0; SubIndex < uniforms.size(); SubIndex++) {
            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptor_sets[Index];
            descriptorWrite.dstBinding = SubIndex;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;

            switch (uniforms[SubIndex]->data->type) {
                case UNIFORM_BUFFER: {
                    auto* bufferInfo = new VkDescriptorBufferInfo();
                    bufferInfo->buffer = uniforms[SubIndex]->uniform[Index]->buffer;
                    bufferInfo->offset = 0;
                    bufferInfo->range = uniforms[SubIndex]->uniform[Index]->buffer_size;

                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrite.pBufferInfo = bufferInfo;
                }
                    break;
                case UNIFORM_TEXTURE: {
                    auto* imageInfo = new VkDescriptorImageInfo();
                    imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                    if (uniforms[SubIndex]->texture == nullptr)
                        debug::error("Attempted To Genereate Unifrom Buffer Without Setting Texture");

                    imageInfo->imageView = uniforms[SubIndex]->texture->image_view;
                    imageInfo->sampler = uniforms[SubIndex]->texture->sampler;

                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.pImageInfo = imageInfo;
                }
                    break;
            }

            descriptorWrites.push_back(descriptorWrite);
        }
        vkUpdateDescriptorSets(shader->device->raw_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void vshader_data::generate(size_t _size) {
    size = _size;

    vector<VkDescriptorSetLayoutBinding> LayoutBindings = vector<VkDescriptorSetLayoutBinding>(uniforms.size());

    for (size_t Index = 0; Index < LayoutBindings.size(); Index++)
        LayoutBindings[Index] = uniforms[Index]->data->binding;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(LayoutBindings.size());
    layoutInfo.pBindings = LayoutBindings.data();

    if (vkCreateDescriptorSetLayout(shader->device->raw_device, &layoutInfo, nullptr, &descriptor_set_layout) != VK_SUCCESS)
        debug::error("failed to create descriptor set layout!");

    descriptor_pool = new vdescriptor_pool(shader->device);
    descriptor_pool->create(shader);

    std::vector<VkDescriptorSetLayout> layouts(size, descriptor_set_layout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool->raw_descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    allocInfo.pSetLayouts = layouts.data();

    descriptor_sets.resize(size);

    VkResult Result = vkAllocateDescriptorSets(shader->device->raw_device, &allocInfo, descriptor_sets.data());

    if (Result != VK_SUCCESS)
        debug::error("failed to allocate descriptor sets!");

    update_descriptors();
}

void vshader_data::change_texture(string _name, vtexture* _texture) {
    for (auto& uni : uniforms) {
        if (uni->data->name == _name) {
            uni->texture = _texture;
            return update_descriptors();
        }
    }

    debug::alert("Shader Data - Failed to find uniform with name " + _name);
}

void vshader_data::bind(vcommand_buffer* _command_buffer, vpipeline* _pipeline) {
    vkCmdBindDescriptorSets(_command_buffer->raw_command_buffers[_command_buffer->record_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->pipeline_layout, 0, 1 , &descriptor_sets[_command_buffer->record_index], 0, nullptr);
}
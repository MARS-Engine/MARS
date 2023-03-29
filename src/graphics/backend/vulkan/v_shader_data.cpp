#include <MARS/graphics/backend/vulkan/v_shader_data.hpp>
#include <MARS/graphics/backend/vulkan/v_texture.hpp>
#include <MARS/graphics/backend/vulkan/v_shader.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>
#include <MARS/graphics/backend/vulkan/v_type_helper.hpp>

using namespace mars_graphics;

void v_uniform::destroy() {
    if (m_buffer == nullptr)
        return;

    m_buffer->destroy();
}

void v_shader_data::bind() {
    vkCmdBindDescriptorSets(cast_graphics<vulkan_backend>()->raw_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::static_pointer_cast<v_pipeline>(m_pipeline)->raw_layout(), 0, 1, &m_descriptor_sets[graphics()->current_frame()], 0, nullptr);
}

void v_shader_data::generate(const std::shared_ptr<pipeline>& _pipeline, const std::shared_ptr<shader>& _shader) {
    m_pipeline = _pipeline;
    m_shader = _shader;

    VkDescriptorPoolCreateInfo poolInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(graphics()->max_frames()),
            .poolSizeCount = static_cast<uint32_t>(std::static_pointer_cast<v_shader>(m_shader)->get_pool_descriptors().size()),
            .pPoolSizes = std::static_pointer_cast<v_shader>(m_shader)->get_pool_descriptors().data(),
    };


    if (vkCreateDescriptorPool(cast_graphics<vulkan_backend>()->device()->raw_device(), &poolInfo, nullptr, &m_descriptor_pool))
        mars_debug::debug::error("MARS - Vulkan - Shader - Failed to create descriptor pool");

    std::vector<VkDescriptorSetLayout> layouts(graphics()->max_frames(), std::static_pointer_cast<v_shader>(m_shader)->raw_uniform_layout());

    VkDescriptorSetAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descriptor_pool,
        .descriptorSetCount = static_cast<uint32_t>(graphics()->max_frames()),
        .pSetLayouts = layouts.data(),
    };

    m_descriptor_sets.resize(graphics()->max_frames());
    if (vkAllocateDescriptorSets(cast_graphics<vulkan_backend>()->device()->raw_device(), &allocInfo, m_descriptor_sets.data()) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Shader Data - Failed to allocate descriptor sets!");

    auto uniforms = _shader->get_uniforms();

    for (size_t i = 0; i < uniforms.size(); i++) {
        if (uniforms[i]->type == MARS_UNIFORM_TYPE_SAMPLER) {
            auto uni = new v_uniform(uniforms[i], i, _shader);
            m_uniforms.insert(std::make_pair(uniforms[i]->name, (uniform*)uni));
            continue;
        }

        std::shared_ptr<buffer> new_buffer = graphics()->create<buffer>();
        new_buffer->create(uniforms[i]->size * sizeof(float), MARS_MEMORY_TYPE_UNIFORM, graphics()->max_frames());

        auto uni = new v_uniform(uniforms[i], i, _shader, new_buffer);
        m_uniforms.insert(std::make_pair(uniforms[i]->name, (uniform*)uni));
    }

    for (auto i = 0; i < graphics()->max_frames(); i++) {
        std::vector<VkDescriptorBufferInfo> buffers;
        std::vector<VkDescriptorImageInfo> textures;
        std::vector<VkWriteDescriptorSet> descriptors;

        for (auto& uni : m_uniforms) {
            if (uni.second->get_data()->type == MARS_UNIFORM_TYPE_SAMPLER)
                continue;
            VkDescriptorBufferInfo info {
                .buffer = std::static_pointer_cast<v_uniform>(uni.second)->get_buffer(i),
                .offset = i * uni.second->get_buffer_size(),
                .range = uni.second->get_data()->size * sizeof(float)
            };

            buffers.push_back(info);
        }

        for (auto& tex : m_textures) {
            VkDescriptorImageInfo info {
                .sampler = std::static_pointer_cast<v_texture>(tex.second)->raw_sampler(),
                .imageView = std::static_pointer_cast<v_texture>(tex.second)->raw_image_view(),
                .imageLayout = MARS2VK(tex.second->layout()),
            };

            textures.push_back(info);
        }

        for (int j = 0; auto& uni : m_uniforms) {
            if (uni.second->get_data()->type == MARS_UNIFORM_TYPE_SAMPLER)
                continue;
            descriptors.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_descriptor_sets[i],
                .dstBinding = static_cast<uint32_t>(uni.second->get_data()->binding),
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &buffers[j++]
            });
        }

        for (int j = 0; auto& tex : m_textures) {
            descriptors.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_descriptor_sets[i],
                .dstBinding = static_cast<uint32_t>(m_uniforms[tex.first]->get_data()->binding),
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &textures[j++]
            });
        }

        vkUpdateDescriptorSets(cast_graphics<vulkan_backend>()->device()->raw_device(), static_cast<uint32_t>(descriptors.size()), descriptors.data(), 0, nullptr);
    }
}

void v_shader_data::destroy() {
    vkDestroyDescriptorPool(cast_graphics<vulkan_backend>()->device()->raw_device(), m_descriptor_pool, nullptr);

    for (auto& uni : m_uniforms)
        uni.second->destroy();
}
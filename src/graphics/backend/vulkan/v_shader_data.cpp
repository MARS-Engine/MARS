#include <MVRE/graphics/backend/vulkan/v_shader_data.hpp>
#include <MVRE/graphics/backend/vulkan/v_texture.hpp>
#include <MVRE/graphics/backend/vulkan/v_shader.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_device.hpp>

using namespace mvre_graphics;

void v_uniform::destroy() {
    for (auto& buffer : m_buffers) {
        buffer->destroy();
        delete buffer;
    }
}

void v_shader_data::bind() {
    vkCmdBindDescriptorSets(instance<v_backend_instance>()->raw_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, ((v_pipeline*)m_pipeline)->raw_layout(), 0, 1, &m_descriptor_sets[instance()->current_frame()], 0, nullptr);
}

void v_shader_data::generate(pipeline* _pipeline, shader* _shader) {
    m_pipeline = _pipeline;
    m_shader = _shader;

    VkDescriptorPoolCreateInfo poolInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(instance()->max_frames()),
            .poolSizeCount = static_cast<uint32_t>(((v_shader*)m_shader)->get_pool_descriptors().size()),
            .pPoolSizes = ((v_shader*)m_shader)->get_pool_descriptors().data(),
    };


    if (vkCreateDescriptorPool(instance<v_backend_instance>()->device()->raw_device(), &poolInfo, nullptr, &m_descriptor_pool))
        mvre_debug::debug::error("MVRE - Vulkan - Shader - Failed to create descriptor pool");

    std::vector<VkDescriptorSetLayout> layouts(instance()->max_frames(),((v_shader*)m_shader)->raw_uniform_layout());

    VkDescriptorSetAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descriptor_pool,
        .descriptorSetCount = static_cast<uint32_t>(instance()->max_frames()),
        .pSetLayouts = layouts.data(),
    };

    mvre_debug::debug::log("Teste");
    mvre_debug::debug::log((std::string)"" + std::to_string(m_shader->get_uniforms().size()));

    m_descriptor_sets.resize(instance()->max_frames());
    auto a = vkAllocateDescriptorSets(instance<v_backend_instance>()->device()->raw_device(), &allocInfo, m_descriptor_sets.data());
    mvre_debug::debug::log((std::string)"" + std::to_string(a));
    if (a != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Shader Data - Failed to allocate descriptor sets!");

    auto uniforms = _shader->get_uniforms();

    for (size_t i = 0; i < uniforms.size(); i++) {
        if (uniforms[i]->type == MVRE_UNIFORM_TYPE_SAMPLER) {
            auto uni = new v_uniform(uniforms[i], i, _shader);
            m_uniforms.insert(std::make_pair(uniforms[i]->name, (uniform*)uni));
            continue;
        }

        std::vector<buffer*> m_buffers(instance()->max_frames());

        for (auto& new_buffer : m_buffers) {
            new_buffer = instance()->instance<buffer>();
            new_buffer->create(uniforms[i]->size * sizeof(float), MVRE_MEMORY_TYPE_UNIFORM);
        }

        auto uni = new v_uniform(uniforms[i], i, _shader, m_buffers);
        m_uniforms.insert(std::make_pair(uniforms[i]->name, (uniform*)uni));
    }

    for (auto i = 0; i < instance()->max_frames(); i++) {
        std::vector<VkDescriptorBufferInfo> buffers;
        std::vector<VkDescriptorImageInfo> textures;
        std::vector<VkWriteDescriptorSet> descriptors;

        for (auto& uni : m_uniforms) {
            if (uni.second->get_data()->type == MVRE_UNIFORM_TYPE_SAMPLER)
                continue;
            VkDescriptorBufferInfo info {
                .buffer = ((v_uniform*)uni.second)->get_buffer(i),
                .offset = 0,
                .range = uni.second->get_data()->size * sizeof(float)
            };

            buffers.push_back(info);

            descriptors.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_descriptor_sets[i],
                .dstBinding = static_cast<uint32_t>(uni.second->get_data()->binding),
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &buffers[buffers.size() - 1]
            });
        }

        for (auto& tex : m_textures) {
            VkDescriptorImageInfo info {
                .sampler = ((v_texture*)tex.second)->raw_sampler(),
                .imageView = ((v_texture*)tex.second)->raw_image_view(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };

            textures.push_back(info);

            descriptors.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_descriptor_sets[i],
                .dstBinding = static_cast<uint32_t>(m_uniforms[tex.first]->get_data()->binding),
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &textures[textures.size() - 1]
            });
        }

        vkUpdateDescriptorSets(instance<v_backend_instance>()->device()->raw_device(), static_cast<uint32_t>(descriptors.size()), descriptors.data(), 0, nullptr);
    }
}

void v_shader_data::destroy() {
    vkDestroyDescriptorPool(instance<v_backend_instance>()->device()->raw_device(), m_descriptor_pool, nullptr);

    for (auto& uni : m_uniforms) {
        uni.second->destroy();
        delete uni.second;
    }
}
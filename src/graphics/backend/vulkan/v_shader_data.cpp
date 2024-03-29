#include <MARS/graphics/backend/vulkan/v_shader_data.hpp>
#include <MARS/graphics/backend/vulkan/v_texture.hpp>
#include <MARS/graphics/backend/vulkan/v_shader.hpp>
#include <MARS/graphics/backend/vulkan/vulkan_backend.hpp>
#include <MARS/graphics/backend/vulkan/v_type_helper.hpp>

using namespace mars_graphics;

void v_uniform::destroy() {
    m_buffer = nullptr;
}

void v_shader_data::bind() {
    vkCmdBindDescriptorSets(cast_graphics<vulkan_backend>()->raw_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.m_pipeline.cast_static<v_pipeline>()->raw_layout(), 0, 1, &m_descriptor_sets[graphics()->current_frame()], 0, nullptr);
}

void v_shader_data::generate(const mars_ref<pipeline>& _pipeline, const mars_ref<shader>& _shader) {
    VkDescriptorPoolCreateInfo poolInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(graphics()->max_frames()),
            .poolSizeCount = static_cast<uint32_t>(m_data.m_shader.cast_static<v_shader>()->get_pool_descriptors().size()),
            .pPoolSizes = m_data.m_shader.cast_static<v_shader>()->get_pool_descriptors().data(),
    };


    auto r = vkCreateDescriptorPool(cast_graphics<vulkan_backend>()->get_device()->raw_device(), &poolInfo, nullptr, &m_descriptor_pool);
    if (r != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Shader - Failed to create descriptor pool");

    std::vector<VkDescriptorSetLayout> layouts(graphics()->max_frames(), m_data.m_shader.cast_static<v_shader>()->raw_uniform_layout());

    VkDescriptorSetAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descriptor_pool,
        .descriptorSetCount = static_cast<uint32_t>(graphics()->max_frames()),
        .pSetLayouts = layouts.data(),
    };

    m_descriptor_sets.resize(graphics()->max_frames());
    if (vkAllocateDescriptorSets(cast_graphics<vulkan_backend>()->get_device()->raw_device(), &allocInfo, m_descriptor_sets.data()) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Shader Data - Failed to allocate descriptor sets!");

    auto uniforms = _shader->get_uniforms();

    for (size_t i = 0; i < uniforms.size(); i++) {
        if (uniforms[i]->type == MARS_UNIFORM_TYPE_SAMPLER) {
            auto uni = new v_uniform(uniforms[i], i, _shader);
            m_uniforms.insert(std::make_pair(uniforms[i]->name, (uniform*)uni));
            continue;
        }

        auto uni = new v_uniform(uniforms[i], i, _shader, graphics()->builder<buffer_builder>().set_size(uniforms[i]->size * sizeof(float)).set_type(MARS_MEMORY_TYPE_UNIFORM).set_frames(graphics()->max_frames()).build());
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

        for (auto& tex : m_data.m_textures) {
            VkDescriptorImageInfo info {
                .sampler = tex.second->cast<v_texture>()->raw_sampler(),
                .imageView = tex.second->cast<v_texture>()->raw_image_view(),
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

        for (int j = 0; auto& tex : m_data.m_textures) {
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

        vkUpdateDescriptorSets(cast_graphics<vulkan_backend>()->get_device()->raw_device(), static_cast<uint32_t>(descriptors.size()), descriptors.data(), 0, nullptr);
    }
}

v_shader_data::~v_shader_data() {
    vkDestroyDescriptorPool(cast_graphics<vulkan_backend>()->get_device()->raw_device(), m_descriptor_pool, nullptr);

    for (auto& uni : m_uniforms)
        uni.second->destroy();
    m_uniforms.clear();
    m_data.m_textures.clear();
}
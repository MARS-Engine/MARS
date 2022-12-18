#include <MVRE/graphics/backend/vulkan/v_shader.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend_instance.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_device.hpp>
#include <MVRE/graphics/backend/vulkan/v_buffer.hpp>

using namespace mvre_graphics;

std::vector<VkPipelineShaderStageCreateInfo> v_shader::get_stages() {
    std::vector<VkPipelineShaderStageCreateInfo> result;

    for (auto& mod : m_v_modules) {
        VkShaderStageFlagBits stage_flag;

        switch (mod.first) {
            case MVRE_SHADER_TYPE_VERTEX:
                stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case MVRE_SHADER_TYPE_FRAGMENT:
                stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            case MVRE_SHADER_TYPE_UNDEFINED:
                stage_flag = VK_SHADER_STAGE_ALL;
                break;
        }

        result.push_back({
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = stage_flag,
            .module = mod.second,
            .pName = "main",
        });
    }

    return result;
}

void v_shader::generate_shader(MVRE_SHADER_TYPE _type, const std::string& _data) {
    VkShaderModuleCreateInfo shader_create_info {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = _data.size(),
            .pCode = reinterpret_cast<const uint32_t*>(_data.data())
    };

    VkShaderModule shader_module;
    if (vkCreateShaderModule(instance<v_backend_instance>()->device()->raw_device(), &shader_create_info, nullptr, &shader_module) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Shader - Failed to create shader module");
    else
        m_v_modules.insert(std::pair<MVRE_SHADER_TYPE, VkShaderModule>(_type, shader_module));

}

bool v_shader::load_resource(const std::string &_path) {
    if (!load_shader_file(_path, ".spv"))
        return false;


    for (auto& m : m_modules) {
        switch (m.first) {
            case MVRE_SHADER_TYPE_VERTEX:
            case MVRE_SHADER_TYPE_FRAGMENT:
                generate_shader(m.first, m.second);
                break;
        }
    }

    std::vector<VkDescriptorSetLayoutBinding> m_descriptors;
    std::vector<VkDescriptorPoolSize> m_pool_descriptors;

    for (size_t i = 0; i < m_uniforms.size(); i++) {
        VkDescriptorSetLayoutBinding new_desc = {
                .binding = static_cast<uint32_t>(i),
                .descriptorCount = 1,
                .pImmutableSamplers = nullptr,
        };

        VkDescriptorPoolSize new_pool_desc = {
            .descriptorCount = static_cast<uint32_t>(instance()->max_frames())
        };

        switch (m_uniforms[i]->type) {
            case MVRE_UNIFORM_TYPE_SAMPLER:
                new_desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                new_desc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                new_pool_desc.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            default:
                new_desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                new_desc.stageFlags = VK_SHADER_STAGE_ALL;
                new_pool_desc.descriptorCount = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
        }

        m_descriptors.push_back(new_desc);
        m_pool_descriptors.push_back(new_pool_desc);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(m_descriptors.size()),
        .pBindings = m_descriptors.data(),
    };

    if (vkCreateDescriptorSetLayout(instance<v_backend_instance>()->device()->raw_device(), &layoutInfo, nullptr, &m_uniform_layout) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Shader - Failed to create descriptor set layout!");

    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(instance()->max_frames()),
        .poolSizeCount = static_cast<uint32_t>(m_pool_descriptors.size()),
        .pPoolSizes = m_pool_descriptors.data(),
    };


    if (vkCreateDescriptorPool(instance<v_backend_instance>()->device()->raw_device(), &poolInfo, nullptr, &m_descriptor_pool))
        mvre_debug::debug::error("MVRE - Vulkan - Shader - Failed to create descriptor pool");

    return true;
}
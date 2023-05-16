#include <MARS/graphics/backend/vulkan/v_shader.hpp>
#include <MARS/graphics/backend/vulkan/vulkan_backend.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>
#include <MARS/graphics/backend/vulkan/v_buffer.hpp>

using namespace mars_graphics;

std::vector<VkPipelineShaderStageCreateInfo> v_shader::get_stages() {
    std::vector<VkPipelineShaderStageCreateInfo> result;

    for (auto& mod : m_v_modules) {
        VkShaderStageFlagBits stage_flag;

        switch (mod.first) {
            case MARS_SHADER_TYPE_VERTEX:
                stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case MARS_SHADER_TYPE_FRAGMENT:
                stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            case MARS_SHADER_TYPE_UNDEFINED:
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

void v_shader::generate_shader(MARS_SHADER_TYPE _type, const std::string& _data) {
    VkShaderModuleCreateInfo shader_create_info {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = _data.size(),
            .pCode = reinterpret_cast<const uint32_t*>(_data.data())
    };

    VkShaderModule shader_module;
    if (vkCreateShaderModule(cast_graphics<vulkan_backend>()->get_device()->raw_device(), &shader_create_info, nullptr, &shader_module) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Shader - Failed to create shader module");
    else
        m_v_modules.insert(std::pair<MARS_SHADER_TYPE, VkShaderModule>(_type, shader_module));

}

bool v_shader::load_shader(const mars_ref<mars_graphics::shader_resource>& _resource) {
    m_data = _resource;
    for (auto& m : m_data->data().modules) {
        switch (m.first) {
            case MARS_SHADER_TYPE_VERTEX:
            case MARS_SHADER_TYPE_FRAGMENT:
                generate_shader(m.first, m.second);
                break;
        }
    }

    std::vector<VkDescriptorSetLayoutBinding> m_descriptors;

    for (auto & m_uniform : m_data->data().uniforms) {
        VkDescriptorSetLayoutBinding new_desc = {
                .binding = static_cast<uint32_t>(m_uniform->binding),
                .descriptorCount = 1,
                .pImmutableSamplers = nullptr,
        };

        VkDescriptorPoolSize new_pool_desc = {
            .descriptorCount = static_cast<uint32_t>(graphics()->max_frames())
        };

        switch (m_uniform->type) {
            case MARS_UNIFORM_TYPE_SAMPLER:
                new_desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                new_desc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                new_pool_desc.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            default:
                new_desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                new_desc.stageFlags = VK_SHADER_STAGE_ALL;
                new_pool_desc.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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

    if (vkCreateDescriptorSetLayout(cast_graphics<vulkan_backend>()->get_device()->raw_device(), &layoutInfo, nullptr, &m_uniform_layout) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Shader - Failed to create descriptor set layout!");

    return true;
}

v_shader::~v_shader() {
    vkDestroyDescriptorSetLayout(cast_graphics<vulkan_backend>()->get_device()->raw_device(), m_uniform_layout, nullptr);

    for (auto& mod : m_v_modules)
        vkDestroyShaderModule(cast_graphics<vulkan_backend>()->get_device()->raw_device(), mod.second, nullptr);
}
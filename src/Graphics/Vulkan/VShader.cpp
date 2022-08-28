#include "VShader.hpp"
#include "VDevice.hpp"
#include "VDescriptorPool.hpp"
#include "Filesystem/FileManager.hpp"
#include "Debug/Debug.hpp"
#include "Misc/StringHelper.hpp"

map<string, ShaderToken> VShader::tokens = {
        { "vertexShader", VERTEX },
        { "fragmentShader", FRAGMENT },
        { "attributes", ATTRIBUTES },
        { "uniforms", UNIFORMS }
};

VUniformData::VUniformData(string value, uint32_t index) {
    type = UNIFORM_BUFFER;
    name = value;

    if (value.find(' ') != string::npos) {
        vector<string> values = Explode(value, ' ');

        if (values.size() >= 2) {
            name = values[0];
            if (values[1] == "TEXTURE")
                type = UNIFORM_TEXTURE;
            else if (values[1] == "BUFFER")
                type = UNIFORM_BUFFER;
            else
                Debug::Alert("Shader - invalid shader type - " + values[1] + " - Falling back to BUFFER");
        }
        else
            Debug::Alert("Shader Error - Invalid shader value - " + value);
    }

    binding.binding = index;
    binding.descriptorCount = 1;
    binding.pImmutableSamplers = nullptr;

    switch (type) {
        case UNIFORM_BUFFER:
            binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            binding.stageFlags = VK_SHADER_STAGE_ALL;
            break;
        case UNIFORM_TEXTURE:
            binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
    }
}

VkShaderModule VShader::LoadShaderModule(vector<uint32_t>& data) const {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    createInfo.codeSize = data.size()  * sizeof(uint32_t);
    createInfo.pCode = data.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device->rawDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        Debug::Error("Vulkan Shader - Failed to create shader module");
    return shaderModule;
}

VShader::VShader(VDevice* _device) {
    device = _device;
}

void VShader::LoadShader(const string& location) {
    vector<string> shaderData;
    if (!FileManager::ReadFile(location, shaderData))
        Debug::Error("Vulkan Shader - Failed to read shader - " + location);

    ShaderToken mode = VERTEX;
    bool modeChange = false;

    for (auto& line : shaderData) {
        if (line[0] == '#') {
            modeChange = true;
            string token = line.substr(1, line.size() - 1);
            if (tokens.find(token) != tokens.end())
                mode = tokens[token];
            else
                Debug::Alert("VShader - Invalid token - " + line + " - in file " += location);
        }

        if (modeChange) {
            modeChange = false;
            continue;
        }

        switch (mode) {
            case VERTEX:
                modules.push_back({ .modulePath = line, .flags = VK_SHADER_STAGE_VERTEX_BIT });
                break;
            case FRAGMENT:
                modules.push_back({ .modulePath = line, .flags = VK_SHADER_STAGE_FRAGMENT_BIT });
                break;
            case ATTRIBUTES:
                break;
            case UNIFORMS:
                if (!line.empty())
                    uniforms.push_back(new VUniformData(line, uniforms.size()));
                break;
        }
    }
    for (auto& m: modules) {
        if (!FileManager::ReadBinaryFile(FileManager::FindFile(FileManager::ShaderLocations(), m.modulePath), m.moduleData))
            return Debug::Error("Vulkan Shader - Failed to read vertex shader - " + m.modulePath);
        m.module = LoadShaderModule(m.moduleData);
    }

    descriptorPool = new VDescriptorPool(device);
    descriptorPool->Create(this);

    UpdateDescriptorLayout();
}

VkDescriptorSetLayout VShader::UpdateDescriptorLayout() {
    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pNext = nullptr;

    setinfo.flags = 0;

    vector<VkDescriptorSetLayoutBinding> bindings;

    for (auto uniform : uniforms)
        bindings.push_back(uniform->binding);

    setinfo.bindingCount = bindings.size();
    setinfo.pBindings = bindings.data();

    vkCreateDescriptorSetLayout(device->rawDevice, &setinfo, nullptr, &layout);
    return layout;
}
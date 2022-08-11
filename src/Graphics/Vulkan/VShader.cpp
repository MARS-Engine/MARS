#include "VShader.hpp"
#include "VDevice.hpp"
#include "VDescriptorPool.hpp"
#include "Filesystem/FileManager.hpp"
#include "Debug/Debug.hpp"
#include "Misc/StringHelper.hpp"

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
            binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
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

    ReadingMode mode = VERTEX;
    bool modeChange = false;
    string vertexShader;
    string fragmentShader;

    for (auto& line : shaderData) {
        if (line[0] == '#') {
            modeChange = true;
            if(line == "#vertexShader")
                mode = VERTEX;
            else if(line == "#fragmentShader")
                mode = FRAGMENT;
            else if(line == "#attributes")
                mode = ATTRIBUTES;
            else if(line == "#uniforms")
                mode = UNIFORMS;
            else
                modeChange = false;
        }

        if (modeChange) {
            modeChange = false;
            continue;
        }

        switch (mode) {
            case VERTEX:
                vertexShader += line;
                break;
            case FRAGMENT:
                fragmentShader += line;
                break;
            case ATTRIBUTES:
                break;
            case UNIFORMS:
                if (!line.empty())
                    uniforms.push_back(new VUniformData(line, uniforms.size()));
                break;
        }
    }

    vector<uint32_t> vertexData;
    vector<uint32_t> fragmentData;

    if (!FileManager::ReadBinaryFile(FileManager::ShaderLocation() + vertexShader, vertexData))
        return Debug::Error("Vulkan Shader - Failed to read vertex shader - " + vertexShader);
    if (!FileManager::ReadBinaryFile(FileManager::ShaderLocation() + fragmentShader, fragmentData))
        return Debug::Error("Vulkan Shader - Failed to read fragment shader - " + fragmentShader);

    vertModule = LoadShaderModule(vertexData);
    fragModule = LoadShaderModule(fragmentData);

    descriptorPool = new VDescriptorPool(device);
    descriptorPool->Create(this);

    layout = GetDescriptorLayout();
}

VkDescriptorSetLayout VShader::GetDescriptorLayout() {
    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pNext = nullptr;

    setinfo.flags = 0;

    vector<VkDescriptorSetLayoutBinding> bindings;

    for (auto uniform : uniforms)
        bindings.push_back(uniform->binding);

    setinfo.bindingCount = bindings.size();
    setinfo.pBindings = bindings.data();

    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(device->rawDevice, &setinfo, nullptr, &layout);
    return layout;
}
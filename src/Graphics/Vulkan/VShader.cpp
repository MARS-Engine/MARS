#include "VShader.hpp"
#include "VDevice.hpp"
#include "Filesystem/FileManager.hpp"
#include "Debug/Debug.hpp"

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
}
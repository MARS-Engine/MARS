#include "vshader.hpp"
#include "vdevice.hpp"
#include "vdescriptor_pool.hpp"
#include "Filesystem/file_manager.hpp"
#include "Debug/debug.hpp"
#include "Misc/string_helper.hpp"

map<string, shader_token> VShader::tokens = {
        { "vertexShader", VERTEX },
        { "fragmentShader", FRAGMENT },
        { "attributes", ATTRIBUTES },
        { "uniforms", UNIFORMS }
};

vuniform_data::vuniform_data(string _value, uint32_t _index) {
    type = UNIFORM_BUFFER;
    name = _value;

    if (_value.find(' ') != string::npos) {
        vector<string> values = explode(_value, ' ');

        if (values.size() >= 2) {
            name = values[0];
            if (values[1] == "TEXTURE")
                type = UNIFORM_TEXTURE;
            else if (values[1] == "BUFFER")
                type = UNIFORM_BUFFER;
            else
                debug::alert("Shader - invalid shader type - " + values[1] + " - Falling back to BUFFER");
        }
        else
            debug::alert("Shader error - Invalid shader value - " + _value);
    }

    binding.binding = _index;
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

VkShaderModule VShader::load_shader_module(vector<uint32_t>& _data) const {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    createInfo.codeSize = _data.size() * sizeof(uint32_t);
    createInfo.pCode = _data.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device->raw_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        debug::error("Vulkan Shader - Failed to create shader module");
    return shaderModule;
}

VShader::VShader(vdevice* _device) {
    device = _device;
}

void VShader::load_shader(const string& _location) {
    vector<string> shaderData;
    if (!FileManager::read_file(FileManager::find_file(FileManager::shader_locations(), _location), shaderData))
        debug::error("Vulkan Shader - Failed to read shader - " + _location);

    shader_token mode = VERTEX;
    bool modeChange = false;

    for (auto& line : shaderData) {
        if (line[0] == '#') {
            modeChange = true;
            string token = line.substr(1, line.size() - 1);
            if (tokens.find(token) != tokens.end())
                mode = tokens[token];
            else
                debug::alert("VShader - Invalid token - " + line + " - in file " += _location);
        }

        if (modeChange) {
            modeChange = false;
            continue;
        }

        switch (mode) {
            case VERTEX:
                modules.push_back({ .module_path = line, .flags = VK_SHADER_STAGE_VERTEX_BIT });
                break;
            case FRAGMENT:
                modules.push_back({ .module_path = line, .flags = VK_SHADER_STAGE_FRAGMENT_BIT });
                break;
            case ATTRIBUTES:
                break;
            case UNIFORMS:
                if (!line.empty())
                    uniforms.push_back(new vuniform_data(line, uniforms.size()));
                break;
        }
    }
    for (auto& m: modules) {
        if (!FileManager::read_binary_file(FileManager::find_file(FileManager::shader_locations(), m.module_path),
                                           m.module_data))
            return debug::error("Vulkan Shader - Failed to read vertex shader - " + m.module_path);
        m.module = load_shader_module(m.module_data);
    }

    descriptor_pool = new vdescriptor_pool(device);
    descriptor_pool->create(this);

    update_descriptor_layout();
}

VkDescriptorSetLayout VShader::update_descriptor_layout() {
    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pNext = nullptr;

    setinfo.flags = 0;

    vector<VkDescriptorSetLayoutBinding> bindings;

    for (auto uniform : uniforms)
        bindings.push_back(uniform->binding);

    setinfo.bindingCount = bindings.size();
    setinfo.pBindings = bindings.data();

    vkCreateDescriptorSetLayout(device->raw_device, &setinfo, nullptr, &layout);
    return layout;
}
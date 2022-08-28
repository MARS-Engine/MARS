#ifndef __VSHADER__
#define __VSHADER__

#include "VTypes.hpp"
#include <string>
#include <vector>
#include <map>

using namespace std;

class VDevice;
class VDescriptorPool;

enum ShaderToken {
    VERTEX,
    FRAGMENT,
    ATTRIBUTES,
    UNIFORMS
};

enum UniformType {
    UNIFORM_BUFFER,
    UNIFORM_TEXTURE,
};

struct VUniformData {
    string name;
    UniformType type;
    VkDescriptorSetLayoutBinding binding;

    VUniformData(string value, uint32_t index);
};

struct ShaderModule {
    string modulePath;
    vector<uint32_t> moduleData;
    VkShaderModule module;
    VkShaderStageFlagBits flags;
};

class VShader {
private:
    VkShaderModule LoadShaderModule(vector<uint32_t>& data) const;
    VkDescriptorSetLayout UpdateDescriptorLayout();
public:
    static map<string, ShaderToken> tokens;
    VDevice* device;
    vector<ShaderModule> modules;
    vector<VUniformData*> uniforms;
    VkDescriptorSetLayout layout;
    VDescriptorPool* descriptorPool;

    VShader(VDevice* device);
    void LoadShader(const string& location);
};

#endif
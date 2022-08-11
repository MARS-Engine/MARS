#ifndef __VSHADER__
#define __VSHADER__

#include "VTypes.hpp"
#include <string>
#include <vector>

using namespace std;

class VDevice;
class VDescriptorPool;

enum ReadingMode {
    VERTEX,
    FRAGMENT,
    ATTRIBUTES,
    UNIFORMS
};

enum UniformType {
    UNIFORM_BUFFER,
    UNIFORM_TEXTURE
};

struct VUniformData {
    string name;
    UniformType type;
    VkDescriptorSetLayoutBinding binding;

    VUniformData(string value, uint32_t index);
};

class VShader {
private:
    VkShaderModule LoadShaderModule(vector<uint32_t>& data) const;
    VkDescriptorSetLayout GetDescriptorLayout();
public:
    VDevice* device;
    VkShaderModule vertModule;
    VkShaderModule fragModule;
    vector<VUniformData*> uniforms;
    VkDescriptorSetLayout layout;
    VDescriptorPool* descriptorPool;

    VShader(VDevice* device);
    void LoadShader(const string& location);
};

#endif
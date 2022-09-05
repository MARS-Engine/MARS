#ifndef __VSHADER__
#define __VSHADER__

#include "vtypes.hpp"
#include <string>
#include <vector>
#include <map>

using namespace std;

class vdevice;
class vdescriptor_pool;

enum shader_token {
    VERTEX,
    FRAGMENT,
    ATTRIBUTES,
    UNIFORMS
};

enum uniform_type {
    UNIFORM_BUFFER,
    UNIFORM_TEXTURE,
};

struct vuniform_data {
    string name;
    uniform_type type;
    VkDescriptorSetLayoutBinding binding;

    vuniform_data(string _value, uint32_t _index);
};

struct shader_module {
    string module_path;
    vector<uint32_t> module_data;
    VkShaderModule module;
    VkShaderStageFlagBits flags;
};

class VShader {
private:
    VkShaderModule load_shader_module(vector<uint32_t>& _data) const;
    VkDescriptorSetLayout update_descriptor_layout();
public:
    static map<string, shader_token> tokens;
    vdevice* device;
    vector<shader_module> modules;
    vector<vuniform_data*> uniforms;
    VkDescriptorSetLayout layout;
    vdescriptor_pool* descriptor_pool;

    VShader(vdevice* _device);
    void load_shader(const string& _location);
};

#endif
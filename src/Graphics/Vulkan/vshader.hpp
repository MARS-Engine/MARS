#ifndef __VSHADER__
#define __VSHADER__

#include "vtypes.hpp"
#include <string>
#include <vector>
#include <map>



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
    std::string name;
    uniform_type type;
    VkDescriptorSetLayoutBinding binding;

    vuniform_data(std::string _value, uint32_t _index);
};

struct shader_module {
    std::string module_path;
    std::vector<uint32_t> module_data;
    VkShaderModule module;
    VkShaderStageFlagBits flags;
};

class VShader {
private:
    VkShaderModule load_shader_module(std::vector<uint32_t>& _data) const;
    VkDescriptorSetLayout update_descriptor_layout();
public:
    static std::map<std::string, shader_token> tokens;
    vdevice* device;
    std::vector<shader_module> modules;
    std::vector<vuniform_data*> uniforms;
    VkDescriptorSetLayout layout;
    vdescriptor_pool* descriptor_pool;

    VShader(vdevice* _device);
    void load_shader(const std::string& _location);
};

#endif
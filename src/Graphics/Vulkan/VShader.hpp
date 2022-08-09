#ifndef __VSHADER__
#define __VSHADER__

#include "VTypes.hpp"
#include <string>
#include <vector>

using namespace std;

class VDevice;

enum ReadingMode {
    VERTEX,
    FRAGMENT,
    ATTRIBUTES,
    UNIFORMS
};

class VShader {
private:
    VkShaderModule LoadShaderModule(vector<uint32_t>& data) const;
public:
    VDevice* device;
    VkShaderModule vertModule;
    VkShaderModule fragModule;

    VShader(VDevice* device);
    void LoadShader(const string& location);
};

#endif
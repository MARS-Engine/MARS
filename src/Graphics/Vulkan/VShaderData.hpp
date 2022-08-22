#ifndef __VSHADERDATA__
#define __VSHADERDATA__

#include "VTypes.hpp"
#include <vector>
#include <map>
#include <string>

using namespace std;

struct VUniformData;
class VDescriptorPool;
class VShader;
class VTexture;
class VBuffer;
class VDevice;
class VCommandBuffer;
class VPipeline;

struct VUniform {
    VUniformData* data;
    VTexture* texture;
    vector<VBuffer*> uniform;
    VDevice* device;
    VmaAllocator allocator;

    VUniform(VUniformData* data, VDevice* device, VmaAllocator& allocator);
    void Generate(size_t bufferSize, size_t framebufferSize);
    void Update(void* data, size_t index);
    void SetTexture(VTexture* texture);
};

class VShaderData {
private:
    size_t size;

    void UpdateDescriptors();
public:
    vector<VUniform*> uniforms;
    VShader* shader;
    VkDescriptorSetLayout descriptorSetLayout;
    VDescriptorPool* descriptorPool;
    vector<VkDescriptorSet> descriptorSets;
    VmaAllocator allocator;

    VShaderData(VShader* shader, VmaAllocator& _allocator);
    VUniform* GetUniform(string name);
    void ChangeTexture(string name, VTexture* texture);
    void Bind(VCommandBuffer* commandBuffer, VPipeline* pipeline);
    void Generate(size_t size);
};

#endif
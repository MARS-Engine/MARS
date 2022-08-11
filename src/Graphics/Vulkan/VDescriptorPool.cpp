#include "VDescriptorPool.hpp"
#include "VDevice.hpp"
#include "VShader.hpp"
#include "../VEngine.hpp"

VDescriptorPool::VDescriptorPool(VDevice* _device) {
    device = _device;
}

void VDescriptorPool::Create(VShader* shader) {
    vector<VkDescriptorPoolSize> pooldesc;

    for (auto uni : shader->uniforms) {
        VkDescriptorPoolSize poolSize{};
        poolSize.descriptorCount = static_cast<uint32_t>(VEngine::FRAME_OVERLAP);

        switch (uni->type) {
            case UNIFORM_BUFFER:
                poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case UNIFORM_TEXTURE:
                poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
        }

        pooldesc.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(pooldesc.size());
    poolInfo.pPoolSizes = pooldesc.data();
    poolInfo.maxSets = static_cast<uint32_t>(VEngine::FRAME_OVERLAP);

    vkCreateDescriptorPool(device->rawDevice, &poolInfo, nullptr, &rawDescriptorPool);

}
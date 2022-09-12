#include "vdescriptor_pool.hpp"
#include "vdevice.hpp"
#include "vshader.hpp"
#include "../vengine.hpp"

vdescriptor_pool::vdescriptor_pool(vdevice* _device) {
    device = _device;
}

void vdescriptor_pool::create(VShader* _shader) {
    std::vector<VkDescriptorPoolSize> pooldesc;

    for (auto uni : _shader->uniforms) {
        VkDescriptorPoolSize poolSize{};
        poolSize.descriptorCount = static_cast<uint32_t>(vengine::frame_overlap);

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
    poolInfo.maxSets = static_cast<uint32_t>(vengine::frame_overlap);

    vkCreateDescriptorPool(device->raw_device, &poolInfo, nullptr, &raw_descriptor_pool);

}
#include <MVRE/graphics/backend/vulkan/v_buffer.hpp>

using namespace mvre_graphics;

void v_buffer::create(size_t _size, MVRE_MEMORY_TYPE _mem_type) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = _size;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    switch (_mem_type) {
        case MVRE_MEMORY_TYPE_VERTEX:
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case MVRE_MEMORY_TYPE_INDEX:
            bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
    }

    if (vkCreateBuffer(instance<v_backend_instance>()->device()->raw_device(), &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Buffer - Failed to create buffer");

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(instance<v_backend_instance>()->device()->raw_device(), m_buffer, &mem_requirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = mem_requirements.size;
    allocInfo.memoryTypeIndex = instance<v_backend_instance>()->device()->find_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(instance<v_backend_instance>()->device()->raw_device(), &allocInfo, nullptr, &m_memory) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Buffer - Failed to allocate buffer memory");
}

void v_buffer::destroy() {
    vkDestroyBuffer(instance<v_backend_instance>()->device()->raw_device(), m_buffer, nullptr);
    vkFreeMemory(instance<v_backend_instance>()->device()->raw_device(), m_memory, nullptr);
}
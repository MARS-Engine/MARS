#include <MARS/graphics/backend/vulkan/v_buffer.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>

using namespace mars_graphics;

void v_buffer::bind() {
    vkBindBufferMemory(cast_graphics<vulkan_backend>()->device()->raw_device(), m_buffer, m_memory, 0);
}

void v_buffer::copy_offset(size_t _offset, size_t _size, void* _data) {
    memcpy(reinterpret_cast<char*>(gpu_data) + _offset, _data, _size);
}

void v_buffer::copy_buffer(v_buffer *_src) {
    VkCommandBuffer commandBuffer = cast_graphics<vulkan_backend>()->get_single_time_command();

    VkBufferCopy copyRegion{};
    copyRegion.size = _src->m_data.size;
    vkCmdCopyBuffer(commandBuffer, _src->m_buffer, m_buffer, 1, &copyRegion);

    cast_graphics<vulkan_backend>()->end_single_time_command(commandBuffer);
}

void v_buffer::copy_data(size_t _index) {
    memcpy(reinterpret_cast<char*>(gpu_data) + _index * m_data.size, m_current_data, m_data.size);
}

void v_buffer::create()  {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = m_data.size * m_data.frames;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    switch (m_data.mem_type) {
        case MARS_MEMORY_TYPE_VERTEX:
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case MARS_MEMORY_TYPE_INDEX:
            bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case MARS_MEMORY_TYPE_TRANSFER:
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            break;
        case MARS_MEMORY_TYPE_UNIFORM:
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
    }

    if (vkCreateBuffer(cast_graphics<vulkan_backend>()->device()->raw_device(), &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Buffer - Failed to create buffer");

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(cast_graphics<vulkan_backend>()->device()->raw_device(), m_buffer, &mem_requirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = mem_requirements.size;
    allocInfo.memoryTypeIndex = cast_graphics<vulkan_backend>()->device()->find_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto r = vkAllocateMemory(cast_graphics<vulkan_backend>()->device()->raw_device(), &allocInfo, nullptr, &m_memory);
    if (r != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Buffer - Failed to allocate buffer memory");

    vkBindBufferMemory(cast_graphics<vulkan_backend>()->device()->raw_device(), m_buffer, m_memory, 0);
    vkMapMemory(cast_graphics<vulkan_backend>()->device()->raw_device(), m_memory, 0, m_data.size * m_data.frames, 0, &gpu_data);
}

v_buffer::~v_buffer() {
    if (gpu_data != nullptr)
        vkUnmapMemory(cast_graphics<vulkan_backend>()->device()->raw_device(), m_memory);

    vkDestroyBuffer(cast_graphics<vulkan_backend>()->device()->raw_device(), m_buffer, nullptr);
    vkFreeMemory(cast_graphics<vulkan_backend>()->device()->raw_device(), m_memory, nullptr);
}
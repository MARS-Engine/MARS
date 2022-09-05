#include "buffer.hpp"
#include "vengine.hpp"
#include "Vulkan/vbuffer.hpp"
#include "Vulkan/vcommand_buffer.hpp"

buffer::buffer(vengine* _engine) {
    engine = _engine;
    staging_buffer = new vbuffer(engine->device, engine->allocator);
    base_buffer = new vbuffer(engine->device, engine->allocator);
}

void buffer::create(size_t _size, buffer_type _type) {
    type = _type;

    if (_size == 0)
        debug::error("Buffer - Attempted to create a buffer with size zero");

    staging_buffer->Create(_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    base_buffer->Create(_size, (type == MEM_BUFF_VERTEX ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : VK_BUFFER_USAGE_INDEX_BUFFER_BIT) | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
}

void buffer::update(void* _data) const {
    staging_buffer->Update(_data);
    base_buffer->CopyBuffer(staging_buffer);
}

void buffer::bind(command_buffer* _command_buffer) const {
    base_buffer->Bind(_command_buffer->base_command_buffer, type == MEM_BUFF_VERTEX ? VBUFFER_VERTEX_BIND : VBUFFER_INDEX_BIND);
}
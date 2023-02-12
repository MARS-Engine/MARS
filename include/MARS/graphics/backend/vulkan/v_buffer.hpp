#ifndef MARS_V_BUFFER_
#define MARS_V_BUFFER_

#include <MARS/graphics/backend/template/buffer.hpp>

#include <vulkan/vulkan.h>

namespace mars_graphics {

    class v_buffer : public buffer {
    private:
        VkBuffer m_buffer;
        VkDeviceMemory m_memory;
        void* gpu_data = nullptr;
        
    public:
        using buffer::buffer;

        inline VkBuffer& vulkan_buffer() { return m_buffer; }
        inline VkDeviceMemory vulkan_memory() { return m_memory; }

        void bind() override;
        void copy_data(size_t _index) override;

        void copy_offset(size_t _offset, size_t _size, void *_data) override;

        void copy_buffer(v_buffer* _src);

        void create(size_t _size, MARS_MEMORY_TYPE _mem_type, size_t _frames) override;
        void destroy() override;
    };
}

#endif
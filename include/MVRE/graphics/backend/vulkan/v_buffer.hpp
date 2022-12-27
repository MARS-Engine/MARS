#ifndef MVRE_V_BUFFER_
#define MVRE_V_BUFFER_

#include <MVRE/graphics/backend/template/buffer.hpp>

#include <vulkan/vulkan.h>

namespace mvre_graphics {

    class v_buffer : public buffer {
    private:
        VkBuffer m_buffer;
        VkDeviceMemory m_memory;
    public:
        using buffer::buffer;

        inline VkBuffer vulkan_buffer() { return m_buffer; }
        inline VkDeviceMemory vulkan_memory() { return m_memory; }

        void bind() override;
        void copy_data(void* data) override;

        void copy_buffer(v_buffer* _src);

        void create(size_t _size, MVRE_MEMORY_TYPE _mem_type) override;
        void destroy() override;
    };
}

#endif
#ifndef __MVRE__V__BUFFER__
#define __MVRE__V__BUFFER__

#include <MVRE/graphics/backend/template/buffer.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_device.hpp>

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

        void bind() override {
            vkBindBufferMemory(instance<v_backend_instance>()->device()->raw_device(), m_buffer, m_memory, 0);
        }

        void copy_data(void* data) override {
            void* gpu_data;
            vkMapMemory(instance<v_backend_instance>()->device()->raw_device(), m_memory, 0, m_size, 0, &data);
            memcpy(gpu_data, data, m_size);
            vkUnmapMemory(instance<v_backend_instance>()->device()->raw_device(), m_memory);
        }

        void create(size_t _size, MVRE_MEMORY_TYPE _mem_type) override;
        void destroy() override;
    };
}

#endif
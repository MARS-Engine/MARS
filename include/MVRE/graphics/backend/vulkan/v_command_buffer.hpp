#ifndef __MVRE__V__COMMAND__BUFFER__
#define __MVRE__V__COMMAND__BUFFER__

#include <MVRE/graphics/backend/template/command_buffer.hpp>
#include <vulkan/vulkan.h>

namespace mvre_graphics {

    class v_command_buffer : public command_buffer {
    private:
        std::vector<VkCommandBuffer> m_command_buffer;
    public:
        using command_buffer::command_buffer;

        inline VkCommandBuffer& raw_command_buffer() { return m_command_buffer[instance()->current_frame()]; }

        void begin() override {
            VkCommandBufferBeginInfo begin_info {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
            };

            if (vkBeginCommandBuffer(m_command_buffer[instance()->current_frame()], &begin_info) != VK_SUCCESS)
                mvre_debug::debug::error("MVRE - Vulkan - Command Buffer - Failed to begin recording command buffer");
        }

        void end() override {
            if (vkEndCommandBuffer(m_command_buffer[instance()->current_frame()]) != VK_SUCCESS)
                mvre_debug::debug::error("MVRE - Vulkan - Command Buffer - Failed to record command buffer");
        }

        void reset() override {
            vkResetCommandBuffer(m_command_buffer[instance()->current_frame()], 0);
        }


        void draw(int first, size_t _count) override {
            vkCmdDraw(m_command_buffer[instance()->current_frame()], _count, 1, 0, 0);
        }

        void draw_indexed(size_t _indices) override {
            vkCmdDrawIndexed(m_command_buffer[instance()->current_frame()], _indices, 1, 0, 0, 0);
        }

        void create();
        void destroy();
    };
}

#endif
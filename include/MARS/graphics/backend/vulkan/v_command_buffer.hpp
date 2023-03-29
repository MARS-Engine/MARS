#ifndef MARS_V_COMMAND_BUFFER_
#define MARS_V_COMMAND_BUFFER_

#include <MARS/graphics/backend/template/command_buffer.hpp>
#include <vulkan/vulkan.h>
#include <vector>

namespace mars_graphics {

    class v_command_buffer : public command_buffer {
    private:
        std::vector<VkCommandBuffer> m_command_buffer;
    public:
        using command_buffer::command_buffer;

        inline VkCommandBuffer& raw_command_buffer() { return m_command_buffer[graphics()->current_frame()]; }

        void begin() override {
            VkCommandBufferBeginInfo begin_info {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
            };

            if (vkBeginCommandBuffer(m_command_buffer[graphics()->current_frame()], &begin_info) != VK_SUCCESS)
                mars_debug::debug::error("MARS - Vulkan - Command Buffer - Failed to begin recording command buffer");
        }

        void end() override {
            if (vkEndCommandBuffer(m_command_buffer[graphics()->current_frame()]) != VK_SUCCESS)
                mars_debug::debug::error("MARS - Vulkan - Command Buffer - Failed to record command buffer");
        }

        void reset() override {
            vkResetCommandBuffer(m_command_buffer[graphics()->current_frame()], 0);
        }


        void draw(int first, size_t _count) override {
            vkCmdDraw(m_command_buffer[graphics()->current_frame()], _count, 1, 0, 0);
        }

        void draw_indexed(size_t _indices) override {
            vkCmdDrawIndexed(m_command_buffer[graphics()->current_frame()], _indices, 1, 0, 0, 0);
        }

        void draw_instanced(size_t _indices, size_t _instances) override {
            vkCmdDrawIndexed(m_command_buffer[graphics()->current_frame()], _indices, _instances, 0, 0, 0);
        }

        void create();
        void destroy();
    };
}

#endif
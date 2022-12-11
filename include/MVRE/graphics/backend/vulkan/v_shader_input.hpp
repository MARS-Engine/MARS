#ifndef __MVRE__V__SHADER__INPUT__
#define __MVRE__V__SHADER__INPUT__

#include <MVRE/graphics/backend/template/shader_input.hpp>
#include <MVRE/graphics/backend/vulkan/v_buffer.hpp>

namespace mvre_graphics {

    class v_shader_input : public shader_input {
    private:
        VkVertexInputBindingDescription m_binding;
        std::vector<VkVertexInputAttributeDescription> m_descriptions;
    public:
        inline VkVertexInputBindingDescription& raw_binding() { return m_binding; }
        inline std::vector<VkVertexInputAttributeDescription>& raw_descriptions() { return m_descriptions; }

        void bind() override {
            for (auto& buffer : m_buffers) {
                auto og_buffer =  (v_buffer*)buffer;

                VkBuffer vertexBuffers[] = { og_buffer->vulkan_buffer() };
                VkDeviceSize offsets[] = {0};

                switch (og_buffer->type()) {
                    case MVRE_MEMORY_TYPE_VERTEX:
                        vkCmdBindVertexBuffers(instance<v_backend_instance>()->raw_command_buffer(), 0, 1, vertexBuffers, offsets);
                        break;
                    case MVRE_MEMORY_TYPE_INDEX:
                        vkCmdBindIndexBuffer(instance<v_backend_instance>()->raw_command_buffer(), vertexBuffers[0], 0, VK_INDEX_TYPE_UINT32);
                        break;
                }
            }
        }

        void load_input(mvre_shader_inputs _inputs) override;

        buffer* add_buffer(size_t _input_size, MVRE_MEMORY_TYPE _input_type) override;
    };
}

#endif
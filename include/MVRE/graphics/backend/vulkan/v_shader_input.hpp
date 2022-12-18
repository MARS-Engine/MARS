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
        using shader_input::shader_input;

        inline VkVertexInputBindingDescription& raw_binding() { return m_binding; }
        inline std::vector<VkVertexInputAttributeDescription>& raw_descriptions() { return m_descriptions; }

        void bind() override;

        void load_input(mvre_shader_inputs _inputs) override;

        buffer* add_buffer(size_t _input_size, MVRE_MEMORY_TYPE _input_type) override;
    };
}

#endif
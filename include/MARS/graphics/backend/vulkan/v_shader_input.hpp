#ifndef MARS_V_SHADER_INPUT_
#define MARS_V_SHADER_INPUT_

#include <MARS/graphics/backend/template/shader_input.hpp>
#include <MARS/graphics/backend/vulkan/v_buffer.hpp>

namespace mars_graphics {

    class v_shader_input : public shader_input {
    private:
        std::vector<VkVertexInputAttributeDescription> m_descriptions;

        void load_input(const std::shared_ptr<mars_shader_inputs>& _inputs) override;

        std::shared_ptr<buffer> add_buffer(size_t _input_size, MARS_MEMORY_TYPE _input_type) override;
    public:
        using shader_input::shader_input;

        ~v_shader_input();

        void bind() override;
    };
}

#endif
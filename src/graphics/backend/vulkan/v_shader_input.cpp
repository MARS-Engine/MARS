#include <MVRE/graphics/backend/vulkan/v_shader_input.hpp>

using namespace mvre_graphics;

void v_shader_input::load_input(mvre_shader_inputs _inputs) {
    int stride = 0;
    for (auto i = 0; i < _inputs.length; i++)
        stride += _inputs.input_data[i].stride;

    for (auto i = 0; i < _inputs.length; i++) {
        VkVertexInputAttributeDescription new_dec{
            .location = static_cast<uint32_t>(i),
            .binding = 0,
            .offset = _inputs.input_data[i].offset,
        };

        switch (_inputs.input_data[i].type) {
            case MVRE_SHADER_INPUT_TYPE_SF_RG:
                new_dec.format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case MVRE_SHADER_INPUT_TYPE_SF_RGB:
                new_dec.format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case MVRE_SHADER_INPUT_TYPE_SF_RGBA:
                new_dec.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
        }

        m_descriptions.push_back(new_dec);

        m_binding = {
            .binding = 0,
            .stride = static_cast<uint32_t>(stride),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
    }
}

buffer* v_shader_input::add_buffer(size_t _input_size, MVRE_MEMORY_TYPE _input_type) {
    auto new_buffer = new v_buffer(instance());
    new_buffer->create(_input_size, _input_type);
    m_buffers.push_back(new_buffer);
    return new_buffer;
}
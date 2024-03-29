#include <MARS/graphics/backend/vulkan/v_shader_input.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>
#include <MARS/graphics/backend/vulkan/vulkan_backend.hpp>

using namespace mars_graphics;

void v_shader_input::bind() {
    for (auto& buffer : m_buffers) {
        auto og_buffer =  buffer->cast<v_buffer>();

        VkBuffer vertexBuffers[] = { og_buffer->vulkan_buffer() };
        VkDeviceSize offsets[] = {0};

        switch (og_buffer->type()) {
            case MARS_MEMORY_TYPE_VERTEX:
                vkCmdBindVertexBuffers(cast_graphics<vulkan_backend>()->raw_command_buffer(), 0, 1, vertexBuffers, offsets);
                break;
            case MARS_MEMORY_TYPE_INDEX:
                vkCmdBindIndexBuffer(cast_graphics<vulkan_backend>()->raw_command_buffer(), vertexBuffers[0], 0, VK_INDEX_TYPE_UINT32);
                break;
        }
    }
}

void v_shader_input::load_input(const mars_shader_inputs& _inputs) {
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
            case MARS_SHADER_INPUT_TYPE_SF_RG:
                new_dec.format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case MARS_SHADER_INPUT_TYPE_SF_RGB:
                new_dec.format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case MARS_SHADER_INPUT_TYPE_SF_RGBA:
                new_dec.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
        }

        m_descriptions.push_back(new_dec);
    }
}

std::shared_ptr<buffer> v_shader_input::add_buffer(size_t _input_size, MARS_MEMORY_TYPE _input_type) {
    auto new_buffer = graphics()->builder<buffer_builder>().set_size(_input_size).set_type(_input_type).build();
    m_buffers.push_back(new_buffer);
    return new_buffer;
}

v_shader_input::~v_shader_input() {
    m_buffers.clear();
}
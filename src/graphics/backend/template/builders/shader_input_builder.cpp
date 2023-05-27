#include <MARS/graphics/backend/template/builders/shader_input_builder.hpp>
#include <MARS/graphics/backend/template/shader_input.hpp>
#include <MARS/graphics/backend/template/buffer.hpp>

using namespace mars_graphics;

std::shared_ptr<buffer> shader_input_builder::add_buffer(size_t _input_size, mars_graphics::MARS_MEMORY_TYPE _input_type) {
    return m_ref->add_buffer(_input_size, _input_type);
}

void shader_input_builder::load_input(const mars_shader_inputs& _inputs) {
    m_ref->load_input(_inputs);
}

std::shared_ptr<shader_input> shader_input_builder::build() {
    base_build();
    return m_ref;
}
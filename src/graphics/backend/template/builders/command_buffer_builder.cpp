#include <MARS/graphics/backend/template/builders/command_buffer_builder.hpp>
#include <MARS/graphics/backend/template/command_buffer.hpp>

using namespace mars_graphics;

command_buffer_builder::command_buffer_builder(const std::shared_ptr<command_buffer>& _ptr) : graphics_builder<command_buffer>(_ptr) {
    m_size = _ptr->graphics()->max_frames();
}

std::shared_ptr<command_buffer> command_buffer_builder::build() {
    base_build();
    m_ref->m_size = m_size;
    m_ref->create();
    return m_ref;
}
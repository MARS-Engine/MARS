#include <MARS/graphics/backend/template/stc/single_time_command.hpp>
#include <MARS/graphics/backend/template/graphics_backend.hpp>
#include <MARS/graphics/backend/template/command_buffer.hpp>

using namespace mars_graphics;

void single_time_command::base_execute() {
    m_executed = true;

    m_command_buffer->end(0);
    m_backend->submit_command(m_command_buffer, 0);
}

single_time_command::single_time_command(const std::shared_ptr<graphics_backend> &_backend) {
    m_backend = _backend;
    m_command_buffer = m_backend->builder<command_buffer_builder>().set_size(1).build();
    m_command_buffer->begin(0);
};
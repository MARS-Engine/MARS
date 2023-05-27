#include <MARS/graphics/backend/template/stc/texture_stc.hpp>
#include <MARS/graphics/backend/template/texture.hpp>

using namespace mars_graphics;

void texture_stc::begin_buffer_copy() {
    m_texture->begin_buffer_copy(get_command_buffer());
}

void texture_stc::finish_buffer_copy() {
    m_texture->finish_buffer_copy(get_command_buffer());
}

void texture_stc::copy_buffer_to_image(const std::shared_ptr<buffer>& _buffer, const mars_math::vector4<uint32_t>& _rect) {
    m_texture->copy_buffer_to_image(get_command_buffer(), _buffer, _rect);
}

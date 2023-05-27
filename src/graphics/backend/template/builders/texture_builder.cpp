#include <MARS/graphics/backend/template/builders/texture_builder.hpp>
#include <MARS/graphics/backend/template/texture.hpp>

using namespace mars_graphics;

texture_builder& texture_builder::initialize() {
    m_ref->set_data(m_data);
    m_ref->initialize();
    m_already_initialize = true;
    return *this;
}

texture_builder& texture_builder::copy_buffer_to_image(const std::shared_ptr<buffer>& _buffer, const mars_math::vector4<uint32_t>& _rect) {
    if (!m_stc) {
        m_stc = std::make_shared<texture_stc>(m_ref->graphics(), m_ref);
        m_stc->begin_buffer_copy();
    }

    m_stc->copy_buffer_to_image(_buffer, _rect);
    return *this;
}

texture_builder& texture_builder::load_from_file(const std::string& _path) {
    m_ref->load_from_file(_path);
    m_already_initialize = true;
    m_complete = true;
    return *this;
}

std::shared_ptr<texture> texture_builder::build() {
    base_build();
    m_ref->set_data(m_data);

    if (m_stc) {
        m_stc->finish_buffer_copy();
        m_stc.reset();
    }
    if (!m_already_initialize)
        m_ref->initialize();
    if (!m_complete)
        m_ref->complete();

    return m_ref;
}
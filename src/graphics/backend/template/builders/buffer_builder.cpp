#include <MARS/graphics/backend/template/builders/buffer_builder.hpp>
#include <MARS/graphics/backend/template/buffer.hpp>

using namespace mars_graphics;

std::shared_ptr<buffer> buffer_builder::build() {
    base_build();
    m_ref->set_data(m_data);
    m_ref->create();

    return m_ref;
}
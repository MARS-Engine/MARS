#include <MARS/graphics/backend/template/builders/window_builder.hpp>
#include <MARS/graphics/backend/template/window.hpp>

using namespace mars_graphics;

std::shared_ptr<window> window_builder::build() {
    base_build();
    m_ref->set_data(m_data);
    m_ref->create();
    return m_ref;
}
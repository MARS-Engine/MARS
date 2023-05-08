#include <MARS/graphics/backend/template/builders/render_pass_builder.hpp>
#include <MARS/graphics/backend/template/render_pass.hpp>

using namespace mars_graphics;

std::shared_ptr<render_pass> render_pass_builder::build() {
    base_build();
    m_ref->set_data(m_data);
    m_ref->create();
    return m_ref;
}
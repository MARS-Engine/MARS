#include <MARS/graphics/backend/template/builders/shader_builder.hpp>
#include <MARS/graphics/backend/template/shader.hpp>
#include <MARS/graphics/backend/template/resources/shader_resource.hpp>

using namespace mars_graphics;
using namespace mars_resources;

shader_builder& shader_builder::load_from_file(const std::string& _path) {
    m_ref->graphics()->resources()->load_resource<shader_resource>(_path, m_shader_resource, m_ref->get_suffix(), m_ref->graphics()->render_type());
    if (m_ref->load_shader(m_shader_resource))
        m_shader_resource->set_shader_ref(mars_ref<shader>(m_ref));
    return *this;
}

std::shared_ptr<shader> shader_builder::build() {
    base_build();
    return m_ref;
}
#include <MARS/graphics/backend/template/builders/shader_data_builder.hpp>
#include <MARS/graphics/backend/template/shader_data.hpp>

using namespace mars_graphics;

std::shared_ptr<shader_data> shader_data_builder::build(const mars_ref<mars_graphics::pipeline> &_pipeline, const mars_ref<mars_graphics::shader> &_shader) {
    base_build();
    m_data.m_shader = _shader;
    m_data.m_pipeline = _pipeline;
    m_ref->set_data(m_data);
    m_ref->generate(_pipeline, _shader);
    return m_ref;
}
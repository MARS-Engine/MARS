#include <MARS/graphics/backend/template/builders/pipeline_builder.hpp>
#include <MARS/graphics/backend/template/pipeline.hpp>

using namespace mars_graphics;

std::shared_ptr<pipeline> pipeline_builder::build() {
    base_build();
    m_ref->set_data(m_data);
    m_ref->create();
    return m_ref;
}
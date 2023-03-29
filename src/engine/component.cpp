#include <MARS/engine/component.hpp>
#include <MARS/engine/mars_object.hpp>

using namespace mars_engine;

void component::set_object(mars_object& _parent) {
    m_object = _parent.get_ptr();
    on_set_object();
}

std::shared_ptr<object_engine> component::engine() const {
    return m_object->engine();
}

std::shared_ptr<mars_graphics::graphics_engine> component::graphics() const {
    return m_object->graphics();
}

transform_3d &component::transform() {
    return m_object->transform();
}
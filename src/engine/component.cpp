#include <MARS/engine/component.hpp>
#include <MARS/engine/mars_object.hpp>

using namespace mars_engine;

void component::set_object(const mars_ref<mars_object>& _parent) {
    m_object = _parent;
    on_set_object();
}

mars_ref<object_engine> component::engine() const {
    return m_object->engine();
}

mars_ref<mars_graphics::graphics_engine> component::graphics() const {
    return m_object->engine()->graphics();
}

transform_3d &component::transform() {
    return m_object->transform();
}
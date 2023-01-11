#include "MARS/engine/engine_object.hpp"
#include "MARS/engine/component.hpp"
#include "MARS/engine/engine_handler.hpp"
#include <MARS/engine/transform_3d.hpp>

using namespace mars_engine;

engine_object::engine_object() {
    m_transform = new transform_3d(this);
}

engine_object::~engine_object() {
    for (auto c : m_components) {
        c->destroy();
        delete c;
    }
    delete m_transform;

    for (auto& c : m_children)
        delete c;
}
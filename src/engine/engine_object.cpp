#include "MVRE/engine/engine_object.hpp"
#include "MVRE/engine/component.hpp"
#include "MVRE/engine/engine_handler.hpp"
#include <MVRE/engine/transform_3d.hpp>

using namespace mvre_engine;

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
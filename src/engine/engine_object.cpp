#include "MARS/engine/engine_object.hpp"
#include "MARS/engine/component.hpp"
#include "MARS/engine/engine_handler.hpp"
#include <MARS/engine/transform_3d.hpp>
#include <MARS/engine/object_bridge.hpp>

using namespace mars_engine;

engine_object::~engine_object() {
    if (m_prev != nullptr)
        m_prev->set_next(m_next);

    if (m_next != nullptr)
        m_next->set_previous(m_prev);

    for (auto c : m_components) {
        c->destroy();
        delete c;
    }

    auto children = m_children.lock_get();

    while (children != nullptr) {
        auto next = children->next();
        delete children;
        children = next;
    }

    for (auto& bridge : m_bridges)
        delete bridge.second;
}
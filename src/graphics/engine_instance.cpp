#include "MVRE/graphics/engine_instance.hpp"
#include "MVRE/graphics/backend/opengl/gl_engine_instance.hpp"

using namespace mvre_graphics;
using namespace mvre_math;
using namespace mvre_graphics_opengl;

void engine_instance::create_with_window() {
    m_window = window();
    m_window.init(get_window_name(), get_window_size(), m_instance_data.type);

    switch (m_instance_data.type) {
        case MVRE_INSTANCE_TYPE_VULKAN:
            break;
        case MVRE_INSTANCE_TYPE_OPENGL:
            backend = new gl_engine_instance();
            break;
    }

    backend->create_with_window(m_window);
}

void engine_instance::update() const {
    backend->update();
}

void engine_instance::prepare_render() const {
    backend->clear();
}

void engine_instance::clean() {
    backend->clean();
    m_window.clean();
}
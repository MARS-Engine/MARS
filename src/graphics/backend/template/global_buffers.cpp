#include <MARS/graphics/backend/template/global_buffers.hpp>

void mars_graphics::global_buffers::destroy() {
    for (auto& buffer : m_global_buffers) {
        buffer.second->destroy();
        delete buffer.second;
    }

    m_global_buffers.clear();
}
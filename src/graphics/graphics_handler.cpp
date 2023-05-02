#include <MARS/graphics/graphics_handler.hpp>
#include <MARS/graphics/graphics_engine.hpp>
#include <MARS/graphics/backend/template/pipeline.hpp>

using namespace std::chrono_literals;
using namespace mars_graphics;

mars_ref<graphics_engine> graphics_handler::engine() {
    return mars_ref<graphics_engine>(m_engine->get_ptr());
}

void graphics_handler::worker_thread() {
    for (auto& callback : *m_engine->get_drawcalls().lock().get()) {
        callback.draw_call();
    }
}
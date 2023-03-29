#include <MARS/graphics/graphics_handler.hpp>
#include <MARS/graphics/graphics_engine.hpp>
#include <MARS/graphics/backend/template/pipeline.hpp>

using namespace std::chrono_literals;
using namespace mars_graphics;

void graphics_handler::worker_thread() {
    m_engine->get_drawcalls()._for([](graphics_draw_call* _call) {
        _call->draw_call();
    });
}
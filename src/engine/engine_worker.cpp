#include <MARS/engine/engine_worker.hpp>
#include <MARS/engine/engine_handler.hpp>

using namespace mars_engine;

void engine_worker::work() {
    while (m_running) {
        m_semaphore.acquire();

        if (!m_running)
            return;

         for (auto& component : m_engine->get_current_components())
            if (!component->completed(true))
                component->callback(component);

        m_semaphore.release();
    }
}
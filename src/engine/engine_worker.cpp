#include <MARS/engine/engine_worker.hpp>
#include <MARS/engine/object_engine.hpp>

using namespace mars_engine;

void engine_worker::work() {
    while (m_running) {
        while (m_running && !m_working) {
            std::unique_lock<std::mutex> l(m_mtx);
            m_cv.wait(l, [&](){ return !m_running || m_working; });
        }

        if (!m_running)
            return;

        auto components = m_engine->get_components(m_layer);

        layer_component_param param {
                .layer_tick = &m_engine->get_layer(m_layer)->m_tick,
                .layers = components,
        };

        for (size_t index = m_index.fetch_add(1); index < components->size(); index = m_index.fetch_add(1)) {
            param.index = index;
            param.component = &components->at(index);
            components->at(index).callback(param);
        }

        m_barriers.arrive_and_wait();
    }
}
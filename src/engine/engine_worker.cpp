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

        long size = std::ceil((long double)components->size() / (long double)m_cores);

        long begin = m_index.fetch_add((long)size);

        m_engine->get_layer(m_layer)->m_callback({
            ._worker = this,
            .layer_tick = &m_engine->get_layer(m_layer)->m_tick,
            .layers = components,
            .being = begin,
            .length = (long)std::min((size_t)size, components->size() - begin)
        });

        m_barriers.arrive_and_wait();
    }
}
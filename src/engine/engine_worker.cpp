#include <MARS/engine/engine_worker.hpp>
#include <MARS/engine/object_engine.hpp>

using namespace mars_engine;

void engine_worker::work(int i) {
    do {
        long size = std::ceil((long double)m_active_components->size() / (long double)m_cores);

        long begin = m_index.fetch_add((long)size);

        m_active_layer->m_callback({
            ._worker = this,
            .layer_tick = &m_active_layer->m_tick,
            .layers = m_active_components,
            .being = begin,
            .length = (long)std::min((size_t)size, m_active_components->size() - begin),
        });

        m_barriers.arrive_and_wait();
    }
    while (m_running);
}
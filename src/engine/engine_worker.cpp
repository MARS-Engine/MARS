#include <MARS/engine/engine_worker.hpp>
#include <MARS/engine/object_engine.hpp>

using namespace mars_engine;

void engine_worker::work(int i) {
    do {
        size_t size = std::ceil((long double)m_active_components->size() / (long double)m_cores);

        size_t begin = m_index.fetch_add(size);
        size = std::min(size, m_active_components->size() - begin);
        m_active_layer->m_callback({
            ._worker = this,
            .layer_tick = &m_active_layer->m_tick,
            .layers = m_active_components->data() + begin,
            .layer_size = m_active_components->size(),
            .offset = begin,
            .length = begin + size > m_active_components->size() ? 0 : size,
            .thread_index = i
        });


        m_barriers.arrive_and_wait();
    }
    while (!m_exit);
}
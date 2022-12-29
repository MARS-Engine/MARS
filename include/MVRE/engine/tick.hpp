#ifndef MVRE_TICK_
#define MVRE_TICK_

#include <cstddef>
#include <MVRE/time/time_helper.hpp>

namespace mvre_engine {
    class tick {
    private:
        float m_tick_rate = 0.0f;
        float tick_per_second = 0.0f;
        float m_last_tick = 0.0f;
    public:
        explicit tick(size_t _tick_rate) {
            m_tick_rate = static_cast<float>(_tick_rate);
            tick_per_second = 1000.0f / m_tick_rate;
        }

        inline bool tick_ready() {
            m_last_tick +=  time_helper::delta_time_ms();
            return  m_last_tick >= tick_per_second;
        }

        inline void reset() {
            m_last_tick = 0.0f;
        }
    };
}

#endif
#ifndef MARS_TICK_
#define MARS_TICK_

#include <cstddef>

namespace mars_engine {
    class tick {
    private:
        std::chrono::_V2::system_clock::time_point _last_time;
        float m_tick_rate = 0.0f;
        float tick_per_second = 0.0f;
    public:
        explicit tick(float _tick_rate) {
            m_tick_rate = _tick_rate;
            tick_per_second = 1000.0f / m_tick_rate;
        }

        inline bool tick_ready() {
            return std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - _last_time).count() >= tick_per_second;
        }

        inline void reset() {
            _last_time = std::chrono::high_resolution_clock::now();
        }
    };
}

#endif
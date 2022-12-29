#ifndef MVRE_TIME_HELPER_
#define MVRE_TIME_HELPER_

#include <chrono>

class time_helper {
private:
    static std::chrono::_V2::system_clock::time_point _last_time;
    static float m_delta_time;
    static float m_delta_time_ms;
public:
    static inline float delta_time() { return m_delta_time; }
    static inline float delta_time_ms() { return m_delta_time_ms; }

    static inline void perpare() {
        _last_time = std::chrono::high_resolution_clock::now();
    }

    static inline void update() {
        auto now = std::chrono::high_resolution_clock::now();
        m_delta_time = std::chrono::duration<float, std::chrono::seconds::period>(now - _last_time).count();
        m_delta_time_ms = std::chrono::duration<float, std::chrono::milliseconds::period>(now - _last_time).count();
    }
};

#endif
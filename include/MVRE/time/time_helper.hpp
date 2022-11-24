#ifndef __MVRE__TIME__HELPER__
#define __MVRE__TIME__HELPER__

#include <chrono>

class time_helper {
private:
    static std::chrono::_V2::system_clock::time_point _last_time;
    static float m_delta_time;
public:
    static inline float delta_time() { return m_delta_time; }

    static inline void perpare() {
        _last_time = std::chrono::high_resolution_clock::now();
    }

    static inline void update() {
        m_delta_time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - _last_time).count();
    }
};

#endif
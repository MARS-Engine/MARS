#include "time_helper.hpp"

std::chrono::_V2::system_clock::time_point time_helper::_last_time;
float time_helper::delta_time;

void  time_helper::prepare() {
    _last_time = std::chrono::high_resolution_clock::now();
}
void time_helper::end() {
    delta_time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - _last_time).count();
}
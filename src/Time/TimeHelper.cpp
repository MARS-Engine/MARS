#include "TimeHelper.hpp"

chrono::_V2::system_clock::time_point TimeHelper::lastTime;
float TimeHelper::deltaTime;

void  TimeHelper::Prepare() {
    lastTime = chrono::high_resolution_clock::now();
}
void TimeHelper::End() {
    deltaTime = chrono::duration<float, chrono::seconds::period>(chrono::high_resolution_clock::now() - lastTime).count();
}
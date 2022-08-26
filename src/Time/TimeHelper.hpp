#ifndef __TIME__HELPER__
#define __TIME__HELPER__

#include <chrono>

using namespace std;

class TimeHelper {
private:
    static chrono::_V2::system_clock::time_point lastTime; 
public:
    static float deltaTime;
    static void  Prepare();
    static void End();
};

#endif
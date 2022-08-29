#ifndef __TIME__HELPER__
#define __TIME__HELPER__

#include <chrono>

using namespace std;

class time_helper {
private:
    static chrono::_V2::system_clock::time_point _last_time;
public:
    static float delta_time;

    static void  prepare();
    static void end();
};

#endif
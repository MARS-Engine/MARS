#ifndef __CORE__
#define __CORE__

#include <thread>

using namespace std;

class Core {
private:
    thread t;

public:
    void Init(int index);
    static void Task(int index);
};

#endif
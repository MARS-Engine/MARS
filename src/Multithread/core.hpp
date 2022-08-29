#ifndef __CORE__
#define __CORE__

#include <thread>

using namespace std;

class core {
private:
    thread _t;
public:
    void init(int index);
    static void work(int index);
};

#endif
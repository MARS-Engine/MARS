#ifndef __CORE__
#define __CORE__

#include <thread>
#include "engine_object.hpp"

using namespace std;

class core {
private:
    int _index;
    thread _t;
public:
    vector<engine_object*> objects;

    void init(int index);
    void work();
};

#endif
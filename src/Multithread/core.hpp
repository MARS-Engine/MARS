#ifndef __CORE__
#define __CORE__

#include <thread>
#include "engine_object.hpp"



class core {
private:
    int _index;
    std::thread _t;
public:
    std::vector<engine_object*> objects;

    void init(int index);
    void work();
};

#endif
#ifndef __PREFAB__MANAGER__
#define __PREFAB__MANAGER__

#include <vector>
#include "Multithread/engine_object.hpp"

using namespace std;

struct prefab_data {
public:
    void* data;
    size_t size;
};

struct prefab {
public:
    prefab_data* data;
    vector<prefab_data*> components;
    vector<prefab*> children;
};

class prefab_manager {
private:
    static prefab_data* object_to_prefab_data(void* object, size_t size);
    static void* prefab_data_to_object(prefab_data* data);
public:
    static vector<prefab*> prefabs;

    static prefab* create_prefab(engine_object* object);
    static engine_object* instance_prefab(prefab* prefab);
};


#endif
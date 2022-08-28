#ifndef HEROSURVIVOR_PREFABMANAGER_HPP
#define HEROSURVIVOR_PREFABMANAGER_HPP


#include <vector>
#include "Multithread/EngineObject.hpp"

using namespace std;

struct PrefabData {
    void* data;
    size_t size;
};

struct Prefab {
    PrefabData* data;
    vector<PrefabData*> components;
    vector<Prefab*> children;
};

class PrefabManager {
private:
    static PrefabData* Data2PrefabData(void* object, size_t size);
    static void* PrefabData2Data(PrefabData* data);
public:
    static vector<Prefab*> prefabs;

    static Prefab* CreatePrefab(EngineObject* object);
    static EngineObject* InstancePrefab(Prefab* prefab);
};


#endif
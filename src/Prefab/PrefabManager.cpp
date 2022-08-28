#include "PrefabManager.hpp"
#include "Multithread/Component.hpp"
#include <cstring>

vector<Prefab*> PrefabManager::prefabs;

PrefabData* PrefabManager::Data2PrefabData(void* object, size_t size) {
    auto data = new PrefabData();
    data->data = malloc(size);
    memcpy(data->data, object, size);
    data->size = size;
    return data;
}

void* PrefabManager::PrefabData2Data(PrefabData* data) {
    void* obj = malloc(data->size);
    memcpy(obj, data->data, data->size);
    return  obj;
}


Prefab* PrefabManager::CreatePrefab(EngineObject* object) {
    auto prefab = new Prefab();

    prefab->data = Data2PrefabData(object, sizeof(EngineObject));

    for (auto c : object->children)
        prefab->children.push_back(CreatePrefab(c));

    for (auto c : object->components)
        prefab->components.push_back(Data2PrefabData(c, c->Size()));

    return prefab;
}

EngineObject* PrefabManager::InstancePrefab(Prefab* prefab) {
    auto obj = (EngineObject*)PrefabData2Data(prefab->data);
    obj->components.clear();

    for (auto c : prefab->components) {
        auto newComponent = (ComponentInterface*)PrefabData2Data(c);
        newComponent->object = obj;
        obj->components.push_back(newComponent);
    }

    obj->children.clear();
    for (auto c : prefab->children)
        obj->AddChild(InstancePrefab(c));

    return  obj;
}
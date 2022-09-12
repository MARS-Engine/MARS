#include "prefab_manager.hpp"
#include "Multithread/component.hpp"
#include <cstring>

std::vector<prefab*> prefab_manager::prefabs;

prefab_data* prefab_manager::object_to_prefab_data(void* object, size_t size) {
    auto data = new prefab_data();
    data->data = malloc(size);
    memcpy(data->data, object, size);
    data->size = size;
    return data;
}

void* prefab_manager::prefab_data_to_object(prefab_data* data) {
    void* obj = malloc(data->size);
    memcpy(obj, data->data, data->size);
    return  obj;
}


prefab* prefab_manager::create_prefab(engine_object* object) {
    auto _prefab = new prefab();

    for (auto c : *object->children)
        _prefab->children.push_back(create_prefab(c));

    for (auto c : *object->components)
        _prefab->components.push_back(object_to_prefab_data(c, c->size()));

    _prefab->data = object_to_prefab_data(object, sizeof(engine_object));

    return _prefab;
}

engine_object* prefab_manager::instance_prefab(prefab* prefab) {
    auto obj = (engine_object*) prefab_data_to_object(prefab->data);

    obj->components = new std::vector<component_interface*>();

    for (auto c : prefab->components) {
        auto new_component = (component_interface*) prefab_data_to_object(c);
        new_component->object = obj;
        obj->components->push_back(new_component);
    }

    obj->children = new std::vector<engine_object*>();
    for (auto c : prefab->children)
        obj->add_child(instance_prefab(c));

    return  obj;
}
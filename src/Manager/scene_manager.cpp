#include "scene_manager.hpp"
#include "Debug/debug.hpp"

void scene::load() { }

scene::scene(const std::string& _name, vengine* _engine) {
    name = _name;
    engine = _engine;
}

std::vector<scene*> scene_manager::scenes;

void scene_manager::add_scene(scene* scene) {
    //TODO: check for repeated names
    scenes.push_back(scene);
}
void scene_manager::load_scene(std::string name) {
    for (auto scene : scenes)
        if (scene->name == name)
            return scene->load();
    debug::alert("Scene Manager - Failed to find scene with name " + name);
}
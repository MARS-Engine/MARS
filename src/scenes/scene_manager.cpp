#include <MARS/scenes/scene_manager.hpp>
#include "MARS/debug/debug.hpp"

using namespace mars_debug;
using namespace mars_scenes;
using namespace pl;

safe_map<std::string, scene*> scene_manager::m_scenes;

void scene_manager::add_scene(const std::string& _scene_name, scene* _scene) {
    m_scenes.lock()->insert(std::make_pair(_scene_name, _scene));
}

void scene_manager::load_scene(const std::string& _scene_name) {
    auto scenes = m_scenes.lock();
    if (!scenes->contains(_scene_name))
        return debug::alert("scene_manager - failed to load scene");
    scenes->at(_scene_name)->load();
}

void scene_manager::clear() {
    m_scenes.lock()->clear();
}
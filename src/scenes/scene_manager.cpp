#include <MVRE/scenes/scene_manager.hpp>
#include "MVRE/debug/debug.hpp"

using namespace mvre_debug;
using namespace mvre_scenes;
using namespace pl;

safe_map<std::string, scene*> scene_manager::m_scenes;

void scene_manager::add_scene(std::string _scene_name, scene* _scene) {
    m_scenes.insert(std::make_pair(_scene_name, _scene));
}

void scene_manager::load_scene(std::string _scene_name) {
    if (!m_scenes.contains(_scene_name))
        return debug::alert("scene_manager - failed to load scene");
    m_scenes[_scene_name]->load();
}
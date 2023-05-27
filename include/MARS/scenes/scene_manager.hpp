#ifndef MARS_SCENE_MANAGER_
#define MARS_SCENE_MANAGER_

#include <string>
#include <pl/safe.hpp>
#include <MARS/engine/singleton.hpp>
#include "scene.hpp"

namespace mars_scenes {

class scene_manager : public mars_engine::singleton {
    private:
        pl::safe<std::map<std::string, scene*>> m_scenes;
    public:

        void add_scene(const std::string& _scene_name, scene* _scene);
        void load_scene(const std::string& _scene_name);

        ~scene_manager();
    };
}

#endif
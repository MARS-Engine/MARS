#ifndef __SCENE__MANAGER__
#define __SCENE__MANAGER__

#include <vector>
#include <string>
#include "Graphics/vengine.hpp"


class scene {
public:
    std::string name;
    vengine* engine;

    scene(const std::string& _name, vengine* _engine);

    virtual void load();
};

class scene_manager {
public:
    static std::vector<scene*> scenes;

    static void add_scene(scene* scene);
    static void load_scene(std::string name);
};

#endif
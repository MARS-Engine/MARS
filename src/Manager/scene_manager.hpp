#ifndef __SCENE__MANAGER__
#define __SCENE__MANAGER__

#include <vector>
#include <string>
#include "Graphics/vengine.hpp"
using namespace std;

class scene {
public:
    string name;
    vengine* engine;

    scene(const string& _name, vengine* _engine);

    virtual void load();
};

class scene_manager {
public:
    static vector<scene*> scenes;

    static void add_scene(scene* scene);
    static void load_scene(string name);
};

#endif
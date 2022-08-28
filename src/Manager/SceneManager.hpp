#ifndef __SCENE__MANAGER__
#define __SCENE__MANAGER__

#include <vector>
#include <string>
#include "Graphics/VEngine.hpp"
using namespace std;

class Scene {
public:
    string name;
    VEngine* engine;

    Scene(const string& name, VEngine* engine);

    virtual void Load();
};

class SceneManager {
public:
    static vector<Scene*> scenes;

    static void AddScene(Scene* scene);
    static void LoadScene(string name);
};

#endif
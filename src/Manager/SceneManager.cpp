#include "SceneManager.hpp"
#include "Debug/Debug.hpp"

void Scene::Load() { }

Scene::Scene(const string& _name, VEngine* _engine) {
    name = _name;
    engine = _engine;
}

vector<Scene*> SceneManager::scenes;

void SceneManager::AddScene(Scene* scene) {
    //TODO: check for repeated names
    scenes.push_back(scene);
}
void SceneManager::LoadScene(string name) {
    for (auto scene : scenes)
        if (scene->name == name)
            return scene->Load();
    Debug::Alert("Scene Manager - Failed to find scene with name " + name);
}
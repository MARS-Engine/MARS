#include "Component.hpp"
#include "EngineObject.hpp"
#include "Manager/MaterialManager.hpp"

Component::Component() {
    material = MaterialManager::GetMaterial("default");
}

void Component::PreLoad() { }
void Component::Load() { }
void Component::PreRender() { }
void Component::Update() { }
void Component::Render() { }
void Component::PostRender() { }
void Component::Clean() { }
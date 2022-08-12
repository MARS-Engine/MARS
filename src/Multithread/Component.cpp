#include "Component.hpp"
#include "EngineObject.hpp"

VEngine* Component::GetEngine() const { return object->engine; }

void Component::PreLoad() { }
void Component::Load() { }
void Component::PreRender() { }
void Component::Update() { }
void Component::Render() { }
void Component::PostRender() { }
void Component::Clean() { }
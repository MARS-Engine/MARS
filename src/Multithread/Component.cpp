#include "Component.hpp"
#include "EngineObject.hpp"

VEngine* Component::GetEngine() const { return object->GetEngine(); }
CommandBuffer* Component::GetCommandBuffer() { return object->GetCommandBuffer(); }

void Component::PreLoad() { }
void Component::Load() { }
void Component::PreRender() { }
void Component::Update() { }
void Component::Render() { }
void Component::PostRender() { }
void Component::Clean() { }
#include "Component.hpp"
#include "EngineObject.hpp"
#include "Manager/MaterialManager.hpp"

size_t ComponentInterface::Size() const { }
void ComponentInterface::PreLoad() { }
void ComponentInterface::Load() { loaded = true; }
void ComponentInterface::PreRender() { }
void ComponentInterface::Update() { }
void ComponentInterface::Render() { }
void ComponentInterface::PostRender() { }
void ComponentInterface::Clean() { }
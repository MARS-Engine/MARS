#include "EngineObject.hpp"
#include "Component.hpp"
#include "Components/Transform.hpp"
#include "Manager/RenderPassManager.hpp"
#include "Manager/MaterialManager.hpp"
#include "Components/Graphics/Camera.hpp"

EngineObject::EngineObject() {
    transform = new Transform(this);
}

void EngineObject::SetEngine(VEngine* _engine) {
    engine = _engine;
    commandBuffer = new CommandBuffer(engine);
    commandBuffer->Create();
    //TODO: add clean code
}

void EngineObject::ExecuteCode(ExecutionCode code) {
    for (auto& component : components) {
        switch (code) {
            case PRE_LOAD:
                component->PreLoad();
                break;
            case LOAD:
                component->Load();
                break;
            case PRE_RENDER:
                if (!component->isRenderer)
                    break;
                for (int i = 0; i < VEngine::FRAME_OVERLAP; i++) {
                    component->GetCommandBuffer()->Begin(i);
                    component->GetCommandBuffer()->LoadDefault(i); //TODO: remove in future or add a conditional
                    component->PreRender();
                    component->GetCommandBuffer()->End();
                }
                break;
            case UPDATE:
                component->Update();
                break;
            case RENDER:
                if (!component->isRenderer)
                    break;
                component->Render();
                if (!component->material->enableTransparency)
                    engine->drawQueue.push_back(component->GetCommandBuffer()->vCommandBuffer->rawCommandBuffers[engine->renderFrame]);
                else {
                    //Maybe implement fast_distance
                    float f = Vector3::Distance(transform->GetPosition(), engine->GetCamera()->transform()->GetPosition());
                    engine->transQueue.insert(pair<float, VkCommandBuffer>(f, component->GetCommandBuffer()->vCommandBuffer->rawCommandBuffers[engine->renderFrame]));
                }
                break;
            case POST_RENDER:
                component->PostRender();
                break;
            case CLEAN:
                component->Clean();
                break;
        }
    }

    for (auto& child : children)
        child->ExecuteCode(code);
}

void EngineObject::AddComponent(Component* component) {
    components.push_back(component);
    component->object = this;
}
void EngineObject::AddChild(EngineObject* child) {
    children.push_back(child);
    child->parent = this;
    child->engine = engine;
}
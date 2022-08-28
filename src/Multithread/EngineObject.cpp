#include "EngineObject.hpp"
#include "Component.hpp"
#include "Components/Transform.hpp"
#include "Manager/RenderPassManager.hpp"
#include "Manager/MaterialManager.hpp"
#include "Components/Graphics/Camera.hpp"

EngineObject::EngineObject() {
    material = MaterialManager::GetMaterial("default");
    transform = new Transform(this);
    children = vector<EngineObject*>();
}

EngineObject::~EngineObject() {
    destroyed = true;

    for (auto& c :components)
        delete c;
    components.resize(0);

    for (auto& c :children)
        delete c;
    children.resize(0);

    delete transform;
}

bool EngineObject::RecursiveCheck(EngineObject* object) {
    if (object->parent == object)
        return false;
    return object->parent == nullptr || object->parent->RecursiveCheck(object);
}

void EngineObject::SetEngine(VEngine* _engine) {
    engine = _engine;
    for (auto c : children)
        c->SetEngine(engine);
}

void EngineObject::ExecuteCode(ExecutionCode code) {
    switch (code) {
        case PRE_RENDER:
            if (find_if(components.begin(), components.end(), [](auto c) { return c->isRenderer; }) == components.end())
                break;

            for (int i = 0; i < VEngine::FRAME_OVERLAP; i++) {
                GetCommandBuffer()->Begin(i);
                GetCommandBuffer()->LoadDefault(i); //TODO: remove in future or add a conditional
                for (auto& component : components)
                    component->PreRender();
                GetCommandBuffer()->End();
            }
            break;
        case RENDER:
            if (commandBuffer == nullptr)
                break;

            if (material->enableTransparency)
                engine->transQueue.push_back({Vector3::Distance(transform->GetPosition(),
                                                                engine->GetCamera()->GetTransform()->GetPosition()), GetCommandBuffer()->vCommandBuffer->rawCommandBuffers[engine->renderFrame]});
            else
                engine->drawQueue.push_back(GetCommandBuffer()->vCommandBuffer->rawCommandBuffers[engine->renderFrame]);
            break;
        default:
            for (auto& component : components) {
                if (destroyed)
                    return;

                switch (code) {
                    case PRE_LOAD:
                        component->PreLoad();
                        break;
                    case LOAD:
                        component->Load();
                        loaded = true;
                        break;
                    case UPDATE:
                        component->Update();
                        break;
                    case POST_RENDER:
                        component->PostRender();
                        break;
                    case CLEAN:
                        component->Clean();
                        break;
                }
            }
            break;
    }

    for (auto& child : children) {
        child->ExecuteCode(code);
        if (destroyed)
            return;
    }

    if (code == UPDATE)
        transform->SetHasUpdated(false);
}

void EngineObject::AddChild(EngineObject* child) {
    if (child == this || !RecursiveCheck(child))
        return Debug::Alert("Attempted to create a child that is itself, a parent or parent of parent");

    children.push_back(child);
    child->parent = this;
    child->engine = engine;

    if (loaded)
        child->ExecuteCode(LOAD);
}
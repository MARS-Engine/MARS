#include "MVRE/engine/engine_object.hpp"
#include "MVRE/engine/component.hpp"
#include "MVRE/engine/engine_handler.hpp"

using namespace mvre_engine;

engine_object::~engine_object() {
    for (auto c : components)
        delete c;
}

void engine_object::execute(MVRE_EXECUTION_CODE _code) {
    for (auto c: components) {
        switch (_code) {
            case PRE_LOAD:
                c->pre_load();
                break;
            case LOAD:
                c->load();
                break;
            case UPDATE:
                c->update();
                break;
            case PRE_RENDER:
                c->pre_render();
                break;
            case RENDER:
                if (c->render_job != nullptr)
                    mvre_executioner::executioner::add_job(mvre_executioner::EXECUTIONER_JOB_PRIORITY_NORMAL, c->render_job);
                break;
            case POST_RENDER:
                c->post_render();
                break;
            case CLEAN:
                c->clean();
                break;
        }
    }
}
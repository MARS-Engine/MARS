#include <MARS/engine/layers/main_layers.hpp>
#include <MARS/engine/component.hpp>

using namespace mars_layers;

std::pair<mars_engine::engine_layer_component*, mars_engine::engine_layer_component*> mars_layers::load_layer_callback(mars_engine::engine_object* _target) {
    mars_engine::engine_layer_component* result = nullptr;
    mars_engine::engine_layer_component* tail = nullptr;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<load_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->load(); };

        if (result == nullptr) {
            result = new_component;
            tail = new_component;
        }
        else {
            tail->next = new_component;
            new_component->previous = tail;
            tail = new_component;
        }
    }

    return { result, tail };
}

std::pair<mars_engine::engine_layer_component*, mars_engine::engine_layer_component*> mars_layers::update_layer_callback(mars_engine::engine_object* _target) {
    mars_engine::engine_layer_component* result = nullptr;
    mars_engine::engine_layer_component* tail = nullptr;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<update_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->update(); };

        if (result == nullptr) {
            result = new_component;
            tail = new_component;
        }
        else {
            tail->next = new_component;
            new_component->previous = tail;
            tail = new_component;
        }
    }

    return { result, tail };
}

std::pair<mars_engine::engine_layer_component*, mars_engine::engine_layer_component*> mars_layers::post_update_layer_callback(mars_engine::engine_object* _target) {
    mars_engine::engine_layer_component* result = nullptr;
    mars_engine::engine_layer_component* tail = nullptr;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<post_update_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->post_update(); };

        if (result == nullptr) {
            result = new_component;
            tail = new_component;
        }
        else {
            tail->next = new_component;
            new_component->previous = tail;
            tail = new_component;
        }
    }

    return { result, tail };
}

std::pair<mars_engine::engine_layer_component*, mars_engine::engine_layer_component*> mars_layers::render_layer_callback(mars_engine::engine_object* _target) {
    mars_engine::engine_layer_component* result = nullptr;
    mars_engine::engine_layer_component* tail = nullptr;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<render_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->send_to_gpu(); mars_executioner::executioner::add_job(target->render_job); };

        if (result == nullptr) {
            result = new_component;
            tail = new_component;
        }
        else {
            tail->next = new_component;
            new_component->previous = tail;
            tail = new_component;
        }
    }

    return { result, tail };
}

std::pair<mars_engine::engine_layer_component*, mars_engine::engine_layer_component*> mars_layers::post_render_layer_callback(mars_engine::engine_object* _target) {
    mars_engine::engine_layer_component* result = nullptr;
    mars_engine::engine_layer_component* tail = nullptr;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<post_render_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->post_render(); };

        if (result == nullptr) {
            result = new_component;
            tail = new_component;
        }
        else {
            tail->next = new_component;
            new_component->previous = tail;
            tail = new_component;
        }
    }

    return { result, tail };
}
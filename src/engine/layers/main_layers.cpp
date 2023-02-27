#include <MARS/engine/layers/main_layers.hpp>
#include <MARS/engine/component.hpp>

using namespace mars_layers;

std::vector<mars_engine::engine_layer_component*> mars_layers::load_layer_callback(mars_engine::engine_object* _target) {
    std::vector<mars_engine::engine_layer_component*> list;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<load_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->load(); };
        list.push_back(new_component);
    }

    return list;
}

std::vector<mars_engine::engine_layer_component*> mars_layers::update_layer_callback(mars_engine::engine_object* _target) {
    std::vector<mars_engine::engine_layer_component*> list;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<update_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->update(); };
        list.push_back(new_component);
    }

    return list;
}

std::vector<mars_engine::engine_layer_component*> mars_layers::post_update_layer_callback(mars_engine::engine_object* _target) {
    std::vector<mars_engine::engine_layer_component*> list;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<post_update_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->post_update(); };
        list.push_back(new_component);
    }

    return list;
}

std::vector<mars_engine::engine_layer_component*> mars_layers::render_layer_callback(mars_engine::engine_object* _target) {
    std::vector<mars_engine::engine_layer_component*> list;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<render_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->send_to_gpu(); mars_executioner::executioner::add_job(target->render_job); };
        list.push_back(new_component);
    }

    return list;
}

std::vector<mars_engine::engine_layer_component*> mars_layers::post_render_layer_callback(mars_engine::engine_object* _target) {
    std::vector<mars_engine::engine_layer_component*> list;

    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<post_render_layer*>(comp);
        if (target == nullptr)
            continue;

        auto new_component = new mars_engine::engine_layer_component();
        new_component->target = target;
        new_component->parent = _target;

        new_component->callback = [target](mars_engine::engine_layer_component*){ target->post_render(); };
        list.push_back(new_component);
    }

    return list;
}
#include <MARS/engine/layers/main_layers.hpp>
#include <MARS/engine/component.hpp>

using namespace mars_layers;

std::vector<mars_engine::engine_layer_component> mars_layers::load_layer_callback(const mars_engine::mars_object& _target) {
    std::vector<mars_engine::engine_layer_component> list;

    for (auto& comp : _target.components()) {
        auto target = dynamic_cast<load_layer*>(comp.get());
        if (target == nullptr)
            continue;

        auto new_component = mars_engine::engine_layer_component();
        new_component.target = target;
        new_component.parent = &_target;

        new_component.callback = [](const mars_engine::layer_component_param& _param){
            static_cast<load_layer*>(_param.component->target)->load();
        };
        list.push_back(new_component);
    }

    return list;
}

std::vector<mars_engine::engine_layer_component> mars_layers::update_layer_callback(const mars_engine::mars_object& _target) {
    std::vector<mars_engine::engine_layer_component> list;

    for (auto& comp : _target.components()) {
        auto target = dynamic_cast<update_layer*>(comp.get());
        if (target == nullptr)
            continue;

        auto new_component = mars_engine::engine_layer_component();
        new_component.target = target;
        new_component.parent = &_target;

        new_component.callback = [](const mars_engine::layer_component_param& _param){ static_cast<update_layer*>(_param.component->target)->update(*_param.layer_tick); };
        list.push_back(new_component);
    }

    return list;
}

std::vector<mars_engine::engine_layer_component> mars_layers::post_update_layer_callback(const mars_engine::mars_object& _target) {
    std::vector<mars_engine::engine_layer_component> list;

    for (auto& comp : _target.components()) {
        auto target = dynamic_cast<post_update_layer*>(comp.get());
        if (target == nullptr)
            continue;

        auto new_component = mars_engine::engine_layer_component();
        new_component.target = target;
        new_component.parent = &_target;

        new_component.callback = [](const mars_engine::layer_component_param& _param){ static_cast<post_update_layer*>(_param.component->target)->post_update(); };
        list.push_back(new_component);
    }

    return list;
}

std::vector<mars_engine::engine_layer_component> mars_layers::update_gpu_callback(const mars_engine::mars_object& _target) {
    std::vector<mars_engine::engine_layer_component> list;

    for (auto& comp : _target.components()) {
        auto target = dynamic_cast<update_gpu*>(comp.get());
        if (target == nullptr)
            continue;

        auto new_component = mars_engine::engine_layer_component();
        new_component.target = target;
        new_component.parent = &_target;

        new_component.callback = [](const mars_engine::layer_component_param& _param){ static_cast<update_gpu*>(_param.component->target)->send_to_gpu(); };
        list.push_back(new_component);
    }

    return list;
}

std::vector<mars_engine::engine_layer_component> mars_layers::post_render_layer_callback(const mars_engine::mars_object& _target) {
    std::vector<mars_engine::engine_layer_component> list;

    for (auto& comp : _target.components()) {
        auto target = dynamic_cast<post_render_layer*>(comp.get());
        if (target == nullptr)
            continue;

        auto new_component = mars_engine::engine_layer_component();
        new_component.target = target;
        new_component.parent = &_target;

        new_component.callback = [](const mars_engine::layer_component_param& _param){ static_cast<post_render_layer*>(_param.component->target)->post_render(); };
        list.push_back(new_component);
    }

    return list;
}
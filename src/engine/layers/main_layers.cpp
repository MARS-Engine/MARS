#include <MARS/engine/layers/main_layers.hpp>
#include <MARS/engine/component.hpp>

using namespace mars_layers;

std::vector<std::function<void(void)>> mars_layers::load_layer_callback(mars_engine::engine_object* _target) {
    std::vector<std::function<void(void)>> result;
    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<load_layer*>(comp);
        if (target != nullptr)
            result.emplace_back([target] { target->load(); });
    }

    return result;
}

std::vector<std::function<void(void)>> mars_layers::update_layer_callback(mars_engine::engine_object* _target) {
    std::vector<std::function<void(void)>> result;
    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<update_layer*>(comp);
        if (target != nullptr)
            result.emplace_back([target] { target->update(); });
    }

    return result;
}

std::vector<std::function<void(void)>> mars_layers::post_update_layer_callback(mars_engine::engine_object* _target) {
    std::vector<std::function<void(void)>> result;
    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<post_update_layer*>(comp);
        if (target != nullptr)
            result.emplace_back([target] { target->post_update(); });
    }

    return result;
}

std::vector<std::function<void(void)>> mars_layers::render_layer_callback(mars_engine::engine_object* _target) {
    std::vector<std::function<void(void)>> result;
    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<render_layer*>(comp);
        if (target != nullptr)
            result.emplace_back([target] { target->send_to_gpu(); mars_executioner::executioner::add_job(target->render_job); });
    }

    return result;
}

std::vector<std::function<void(void)>> mars_layers::post_render_layer_callback(mars_engine::engine_object* _target) {
    std::vector<std::function<void(void)>> result;
    for (auto& comp : _target->components()) {
        auto target = dynamic_cast<post_render_layer*>(comp);
        if (target != nullptr)
            result.emplace_back([target] { target->post_render(); });
    }

    return result;
}
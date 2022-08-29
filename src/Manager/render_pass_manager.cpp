#include "render_pass_manager.hpp"

vector<vrender_pass*> render_pass_manager::renderPasses;

vrender_pass* render_pass_manager::get_render_pass(const string& name, vengine* engine, render_pass_data type) {
    for (auto renderPass : renderPasses)
        if (renderPass->name == name)
            return renderPass;
    auto renderPass = new vrender_pass(engine->allocator, engine->device, type);
    renderPasses.push_back(renderPass);
    renderPass->name = name;
    return renderPass;
}
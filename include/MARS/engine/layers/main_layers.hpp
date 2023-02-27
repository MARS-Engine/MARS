#ifndef MARS_MAIN_LAYERS_
#define MARS_MAIN_LAYERS_

#include <MARS/engine/engine_object.hpp>
#include <MARS/engine/engine_handler.hpp>
#include <MARS/graphics/backend/template/command_buffer.hpp>
#include <MARS/graphics/material.hpp>

namespace mars_layers {
    /**
     * Layer containing main functions, exp: load, update, render, etc...
     */

    class load_layer {
    public:
        virtual void load() { }
    };

    class update_layer {
    public:
        virtual void update() { }
    };

    class post_update_layer {
    public:
        virtual void post_update() { }
    };

    class render_layer {
    public:
        mars_executioner::executioner_job* render_job = nullptr;
        mars_graphics::material* render_material = nullptr;

        virtual void send_to_gpu() { }

        ~render_layer() {
            delete render_job;
        }
    };

    class post_render_layer {
    public:
        virtual void post_render() { }
    };

    std::vector<mars_engine::engine_layer_component*> load_layer_callback(mars_engine::engine_object* _target);
    std::vector<mars_engine::engine_layer_component*> update_layer_callback(mars_engine::engine_object* _target);
    std::vector<mars_engine::engine_layer_component*> post_update_layer_callback(mars_engine::engine_object* _target);
    std::vector<mars_engine::engine_layer_component*> render_layer_callback(mars_engine::engine_object* _target);
    std::vector<mars_engine::engine_layer_component*> post_render_layer_callback(mars_engine::engine_object* _target);
}

#endif
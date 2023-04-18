#ifndef MARS_MAIN_LAYERS_
#define MARS_MAIN_LAYERS_

#include <MARS/engine/mars_object.hpp>
#include <MARS/engine/object_engine.hpp>
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
        virtual void update(const mars_engine::tick& _tick) { }
    };

    class post_update_layer {
    public:
        virtual void post_update() { }
    };

    class update_gpu {
    public:
        virtual void send_to_gpu() { }
    };

    class post_render_layer {
    public:
        virtual void post_render() { }
    };

    void load_layer_callback(mars_engine::layer_component_param&& _param);
    void update_layer_callback(mars_engine::layer_component_param&& _param);
    void post_update_layer_callback(mars_engine::layer_component_param&& _param);
    void update_gpu_callback(mars_engine::layer_component_param&& _param);
    void post_render_layer_callback(mars_engine::layer_component_param&& _param);
}

#endif
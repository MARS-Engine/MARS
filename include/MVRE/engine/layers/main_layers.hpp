#ifndef __MVRE__MAIN__LAYERS__
#define __MVRE__MAIN__LAYERS__

#include <MVRE/engine/engine_object.hpp>
#include <MVRE/engine/engine_handler.hpp>
#include <MVRE/graphics/backend/template/command_buffer.hpp>
#include <MVRE/graphics/material.hpp>

namespace mvre_layers {
    /**
     * Layer containing main functions, exp: load, update, render, etc...
     */

    class load_layer {
    public:
        virtual void load() { }
    };

    class update_layer {
    public:
        virtual void pre_update() { }
        virtual void update() { }
        virtual void post_update() { }
    };

    class render_layer {
    public:
        mvre_executioner::executioner_job* render_job = nullptr;
        mvre_graphics::material* render_material = nullptr;

        ~render_layer() {
            delete render_job;
        }

        virtual void pre_render() { }
        virtual void post_render() { }
    };

    void load_layer_callback(mvre_engine::engine_layers* _layer, int _thread);

    void update_layer_callback(mvre_engine::engine_layers* _layer, int _thread);

    void render_layer_callback(mvre_engine::engine_layers* _layer, int _thread);

}

#endif
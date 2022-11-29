#ifndef __MVRE__MAIN__LAYERS__
#define __MVRE__MAIN__LAYERS__

#include <MVRE/engine/engine_layer.hpp>
#include <MVRE/engine/engine_object.hpp>

namespace mvre_layers {
    /**
     * Layer containing main functions, exp: load, update, render, etc...
     */
    class component_main_layer {
    public:
        virtual void load() { }

        virtual void pre_update() { }
        virtual void update() { }
        virtual void post_update() { }

        virtual void pre_render() { }
        virtual void post_render() { }
    };

    class load_layer : public mvre_engine::engine_layer {
    public:
        void process_engine_list(const std::vector<mvre_engine::engine_object*>& _process_order) override;
    };

    class update_layer : public mvre_engine::engine_layer {
    public:
        void process_engine_list(const std::vector<mvre_engine::engine_object*>& _process_order) override;
    };

    class render_layer : public mvre_engine::engine_layer {
    public:
        void process_engine_list(const std::vector<mvre_engine::engine_object*>& _process_order) override;
    };
}

#endif
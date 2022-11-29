#ifndef __MVRE__CAMERA__COMPONENT__
#define __MVRE__CAMERA__COMPONENT__

#include <MVRE/engine/component.hpp>
#include <MVRE/engine/layers/main_layers.hpp>

namespace mvre_component {
    class camera_updater : public mvre_engine::component<camera_updater>, public mvre_layers::component_main_layer {
    public:
        void pre_update() override;
    };
}

#endif
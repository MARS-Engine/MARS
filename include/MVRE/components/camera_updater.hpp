#ifndef __MVRE__CAMERA__COMPONENT__
#define __MVRE__CAMERA__COMPONENT__

#include <MVRE/engine/component.hpp>
#include <MVRE/engine/layers/main_layers.hpp>

namespace mvre_component {

    enum MVRE_CAMERA_MODE {
        MVRE_CAMERA_MODE_PERSPECTIVE,
        MVRE_CAMERA_MODE_ORTHOGRAPIC
    };

    class camera_updater : public mvre_engine::component<camera_updater>, public mvre_layers::update_layer {
    private:
        MVRE_CAMERA_MODE m_camera_mode = MVRE_CAMERA_MODE_PERSPECTIVE;
    public:
        inline void set_camera_mode(MVRE_CAMERA_MODE _mode) { m_camera_mode = _mode; }

        void pre_update() override;
    };
}

#endif
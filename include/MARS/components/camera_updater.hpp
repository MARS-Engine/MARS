#ifndef MARS_CAMERA_COMPONENT_
#define MARS_CAMERA_COMPONENT_

#include <MARS/engine/component.hpp>
#include <MARS/engine/layers/main_layers.hpp>

namespace mars_component {

    enum MARS_CAMERA_MODE {
        MARS_CAMERA_MODE_PERSPECTIVE,
        MARS_CAMERA_MODE_ORTHOGRAPHIC
    };

    class camera_updater : public mars_engine::component, public mars_layers::update_layer, public mars_layers::post_update_layer {
    private:
        MARS_CAMERA_MODE m_camera_mode = MARS_CAMERA_MODE_PERSPECTIVE;
        float m_fov = 90;
    public:
        inline void set_camera_mode(MARS_CAMERA_MODE _mode) { m_camera_mode = _mode; }
        inline void set_fov(float _fov) { m_fov = _fov; }

        void post_update() override;
    };
}

#endif
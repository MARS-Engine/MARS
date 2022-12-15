#include <MVRE/components/camera_updater.hpp>

using namespace mvre_component;
using namespace mvre_math;

void camera_updater::pre_update() {
    g_instance()->get_camera().set_view(matrix4<float>::look_at_lh({ 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 }));

    switch (m_camera_mode) {
        case MVRE_CAMERA_MODE_PERSPECTIVE:
            g_instance()->get_camera().set_projection(matrix4<float>::perspective_fov_lh(90, 1920, 1080, 0.1f ,1000));
            break;
        case MVRE_CAMERA_MODE_ORTHOGRAPIC:
            g_instance()->get_camera().set_projection(matrix4<float>::ortho_lh(0.f, 1920.f, 1080.f, 0.f, 0.1f , 1000.f));
            break;
    }

    g_instance()->get_camera().set_view_proj(g_instance()->get_camera().get_projection() * g_instance()->get_camera().get_view());
}
#include <MARS/components/camera_updater.hpp>

using namespace mars_component;
using namespace mars_math;

void camera_updater::post_update() {
    g_instance()->get_camera().set_view(matrix4<float>::look_at_lh(transform().position(), transform().position() + transform().rotation() * vector3<float>::forward(), vector3<float>::up()));

    auto size = g_instance()->backend()->get_window()->size();

    switch (m_camera_mode) {
        case MARS_CAMERA_MODE_PERSPECTIVE:
            g_instance()->get_camera().set_projection(matrix4<float>::perspective_fov_lh(m_fov, size.x(), size.y(), 0.1f ,1000));
            break;
        case MARS_CAMERA_MODE_ORTHOGRAPHIC:
            g_instance()->get_camera().set_projection(matrix4<float>::ortho_lh(0.f, size.x(), size.y(), 0.f, 0.1f , 1000.f));
            break;
    }

    g_instance()->get_camera().set_view_proj(g_instance()->get_camera().get_projection() * g_instance()->get_camera().get_view());
    g_instance()->get_camera().position(transform().get_world_position());
}
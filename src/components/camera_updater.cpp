#include <MARS/components/camera_updater.hpp>

using namespace mars_component;
using namespace mars_math;

void camera_updater::post_update() {
    graphics()->get_camera().set_view(matrix4<float>::look_at_lh(transform().position(), transform().position() + transform().rotation() * vector3<float>::forward(), vector3<float>::up()));

    auto size = graphics()->backend()->get_window()->size();

    switch (m_camera_mode) {
        case MARS_CAMERA_MODE_PERSPECTIVE:
            graphics()->get_camera().set_projection(matrix4<float>::perspective_fov_lh(m_fov, size.x(), size.y(), 0.1f , 1000));
            break;
        case MARS_CAMERA_MODE_ORTHOGRAPHIC:
            graphics()->get_camera().set_projection(matrix4<float>::ortho_lh(0.f, size.x(), size.y(), 0.f, 0.1f , 1000.f));
            break;
    }

    graphics()->get_camera().set_view_proj(
            graphics()->get_camera().get_projection() * graphics()->get_camera().get_view());
    graphics()->get_camera().position(transform().get_world_position());
}
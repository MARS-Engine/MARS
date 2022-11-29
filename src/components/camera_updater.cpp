#include <MVRE/components/camera_updater.hpp>

using namespace mvre_component;
using namespace mvre_math;

void camera_updater::pre_update() {
    g_instance()->get_camera().set_view(matrix4<float>::look_at_lh({ 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 }));
    g_instance()->get_camera().set_projection(matrix4<float>::perspective_fov_lh(90, 1920, 1080, 0.1f ,1000));
    g_instance()->get_camera().set_view_proj(g_instance()->get_camera().get_projection() * g_instance()->get_camera().get_view());
}
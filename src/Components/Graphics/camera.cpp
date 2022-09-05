#include "camera.hpp"
#include "Graphics/window.hpp"

void Camera::update_camera() {
    perspective = matrix4::perspective_lh(90.f, 1280.f / 720.f, 0.1f, 200.f);
    view = matrix4::look_at_lh(get_transform()->get_world_position(), get_transform()->get_world_position() +
            get_transform()->get_rotation() * vector3::forward(), vector3::up());
    ortho = matrix4::ortho_lh(_ortho_size.x, _ortho_size.y, _ortho_size.z, _ortho_size.w, 0.f, 1000.f);
    projection_view = (_camera_mode == PERSPECTIVE ? perspective : ortho) * view;
}

camera_mode Camera::get_mode() {
    return _camera_mode;
}
void Camera::set_mode(camera_mode _mode) {
    _mode = _mode;
    update_camera();
}

vector4 Camera::get_ortho_size() {
    return _ortho_size;
}
void Camera::set_ortho_size(vector4 _size) {
    _ortho_size = _size;
    update_camera();
}

void Camera::pre_load() {
    get_engine()->cameras.push_back(this);
    update_camera();
}

void Camera::update() {
    if (get_transform()->updated() || get_transform()->has_updated())
        update_camera();
}
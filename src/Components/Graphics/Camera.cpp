#include "Camera.hpp"
#include "Graphics/Window.hpp"

void Camera::UpdateCam() {
    Perspective = Matrix4::PerspectiveLH(90.f, 1280.f / 720.f, 0.1f, 200.f);
    View = Matrix4::LookAtLH(transform()->GetWorldPosition(), transform()->GetWorldPosition() + transform()->GetRotation() * Vector3::Forward(), Vector3::Up());
    Ortho = Matrix4::OrthoLH(orthoSize.x, orthoSize.y, orthoSize.z, orthoSize.w, 0.f, 1000.f);
    ProjectionView = (mode == PERSPECTIVE ? Perspective : Ortho) * View;
}

CAMERA_MODE Camera::GetMode() {
    return mode;
}
void Camera::SetMode(CAMERA_MODE _mode) {
    mode = _mode;
    UpdateCam();
}

Vector4 Camera::GetOrthoSize() {
    return orthoSize;
}
void Camera::SetOrthoSize(Vector4 size) {
    orthoSize = size;
    UpdateCam();
}

void Camera::PreLoad() {
    GetEngine()->cameras.push_back(this);
    UpdateCam();
}

void Camera::Update() {
    if (transform()->Updated())
        UpdateCam();
}
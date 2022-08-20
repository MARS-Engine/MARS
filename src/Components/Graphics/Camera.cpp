#include "Camera.hpp"
#include "Graphics/Window.hpp"

void Camera::UpdateCam() {
    Projection = Matrix4::PerspectiveFovLH(90.f, 1280.f, 720.f, 0.1f, 200.f);
    View = Matrix4::LookAtLH(transform()->GetWorldPosition(), transform()->GetWorldPosition() + transform()->GetRotation() * Vector3::Forward(), Vector3::Up());
    ProjectionView = Projection * View;
    Ortho = Matrix4::OrthoLH(0, GetEngine()->window->windowSize.width, GetEngine()->window->windowSize.height, 0, 0, 1000000);
}

void Camera::PreLoad() {
    GetEngine()->cameras.push_back(this);
    UpdateCam();
}

void Camera::Update() {
    UpdateCam();
}
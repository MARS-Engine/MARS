#include "Camera.hpp"

void Camera::PreLoad() {
    GetEngine()->cameras.push_back(this);
}

void Camera::Update() {
    Projection = Matrix4::PerspectiveFovLH(90.f, 1280.f, 720.f, 0.1f, 200.f);
    View = Matrix4::LookAtLH(transform()->GetWorldPosition(), transform()->GetWorldPosition() + transform()->GetRotation() * Vector3::Forward(), Vector3::Up());
    ProjectionView = Projection * View;
}

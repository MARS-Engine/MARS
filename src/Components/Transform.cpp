#include "Transform.hpp"
#include "Multithread/EngineObject.hpp"

Transform::Transform(EngineObject* _object) {
    object = _object;
    position = Vector3::Zero();
    scale = Vector3::One();
    rotation = Quaternion();
    UpdateTransform();
}

void Transform::UpdateTransform() {
    transform = Matrix4::Translate(position) * Matrix4::FromQuaternion(rotation) * Matrix4::Scale(scale);
    if (object != nullptr && object->parent != nullptr)
        transform = object->parent->transform->GetTransform() * transform;
    updated = false;
}

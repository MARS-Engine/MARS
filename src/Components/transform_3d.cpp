#include "transform_3d.hpp"
#include "Multithread/engine_object.hpp"

transform_3d::transform_3d(engine_object* _object) {
    object = _object;
    _position = vector3();
    _scale = vector3::one();
    _rotation = quaternion();
    update_transform();
}

void transform_3d::update_transform() {
    _transform_mat = matrix4::translate(_position) * matrix4::from_quaternion(_rotation) * matrix4::scale(_scale);
    _updated = false;
    _has_updated = true;

    if (object != nullptr && object->parent != nullptr)
        _transform_mat = object->parent->transform->get_transform() * _transform_mat;
}

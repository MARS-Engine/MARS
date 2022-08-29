#ifndef __TRANSFORM__
#define __TRANSFORM__

#include "Math/vector3.hpp"
#include "Math/quaternion.hpp"
#include "Math/matrix4.hpp"
#include "Multithread/engine_object.hpp"

class transform_3d {
private:
    vector3 _position;
    vector3 _scale;
    quaternion _rotation;
    matrix4 _transform_mat;
    bool _updated;
    bool _has_updated;

    void update_transform();
public:
    engine_object* object;

    transform_3d(engine_object* _object);

    inline void set_has_updated(bool _value) { _has_updated = _value; }
    inline bool has_updated() const { return _has_updated || object && object->parent && object->parent->transform->has_updated(); }
    inline bool updated() const { return _updated || object && object->parent && object->parent->transform->updated(); }

    inline vector3 get_world_position() { return (object != nullptr && object->parent != nullptr ? object->parent->transform->get_world_position() : vector3()) + _position; };
    inline vector3 get_position() { return _position; };
    inline vector3 get_scale() { return _scale; };
    inline quaternion get_rotation() { return _rotation; };
    inline matrix4 get_transform() { if (updated() || (object && object->parent &&
                                                       object->parent->transform->has_updated())) update_transform(); return _transform_mat; };

    inline void set_position(vector3 _value) { if (_position != _value) _updated = true; _position = _value; };
    inline void set_scale(vector3 _value) { if (_scale != _value) _updated = true; _scale = _value; };
    inline void set_rotation(quaternion _value) { if (_rotation != _value) _updated = true; _rotation = _value; };
};

#endif
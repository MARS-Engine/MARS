#ifndef __TRANSFORM__
#define __TRANSFORM__

#include "Math/Vector3.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Matrix4.hpp"
#include "Multithread/EngineObject.hpp"

class Transform {
private:
    Vector3 position;
    Vector3 scale;
    Quaternion rotation;
    Matrix4 transform;

    void UpdateTransform();
public:
    EngineObject* object;

    Transform(EngineObject* object);

    inline Vector3 GetPosition() { return position; };
    inline Vector3 GetScale() { return scale; };
    inline Quaternion GetRotation() { return rotation; };
    inline Matrix4 GetTransform() { return object == nullptr || object->parent == nullptr ? transform : object->parent->transform->GetTransform() * transform; };

    inline Vector3 SetPosition(Vector3 v) { position = v; UpdateTransform(); };
    inline Vector3 SetScale(Vector3 v) { scale = v; UpdateTransform(); };
    inline Quaternion SetRotation(Quaternion v) { rotation = v; UpdateTransform(); };
};

#endif
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
    bool updated;

    void UpdateTransform();
public:
    EngineObject* object;

    Transform(EngineObject* object);

    inline bool Updated() const { return updated || !(object == nullptr || object->parent == nullptr) && object->parent->transform->Updated(); }

    inline Vector3 GetWorldPosition() { return (object != nullptr && object->parent != nullptr ? object->parent->transform->GetWorldPosition() : Vector3()) + position; };
    inline Vector3 GetPosition() { return position; };
    inline Vector3 GetScale() { return scale; };
    inline Quaternion GetRotation() { return rotation; };
    inline Matrix4 GetTransform() { if (Updated()) UpdateTransform(); return transform; };

    inline Vector3 SetPosition(Vector3 v) { position = v; updated = true; };
    inline Vector3 SetScale(Vector3 v) { scale = v; updated = true; };
    inline Quaternion SetRotation(Quaternion v) { rotation = v; updated = true; };
};

#endif
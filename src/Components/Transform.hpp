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
    bool hasUpdated;

    void UpdateTransform();
public:
    EngineObject* object;

    Transform(EngineObject* object);

    inline void SetHasUpdated(bool value) { hasUpdated = value; }
    inline bool HasUpdated() const { return hasUpdated || object && object->parent && object->parent->transform->HasUpdated(); }
    inline bool Updated() const { return updated || object && object->parent && object->parent->transform->Updated(); }

    inline Vector3 GetWorldPosition() { return (object != nullptr && object->parent != nullptr ? object->parent->transform->GetWorldPosition() : Vector3()) + position; };
    inline Vector3 GetPosition() { return position; };
    inline Vector3 GetScale() { return scale; };
    inline Quaternion GetRotation() { return rotation; };
    inline Matrix4 GetTransform() { if (Updated() || (object && object->parent && object->parent->transform->HasUpdated())) UpdateTransform(); return transform; };

    inline void SetPosition(Vector3 v) { if (position != v) updated = true; position = v; };
    inline void SetScale(Vector3 v) { if (scale != v) updated = true;  scale = v; };
    inline void SetRotation(Quaternion v) { if (rotation != v) updated = true;  rotation = v; };
};

#endif
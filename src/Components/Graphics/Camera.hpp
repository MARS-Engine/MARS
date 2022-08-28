#ifndef __CAMERA__
#define __CAMERA__

#include "Multithread/Component.hpp"

enum CAMERA_MODE {
    PERSPECTIVE,
    ORTHO
};

class Camera : public Component<Camera> {
private:
    CAMERA_MODE mode;
    Vector4 orthoSize;

    void UpdateCam();
public:
    Matrix4 View;
    Matrix4 Perspective;
    Matrix4 ProjectionView;
    Matrix4 Ortho;

    CAMERA_MODE GetMode();
    void SetMode(CAMERA_MODE mode);

    Vector4 GetOrthoSize();
    void SetOrthoSize(Vector4 size);

    void PreLoad() override;
    void Update() override;
};

#endif
#ifndef __CAMERA__
#define __CAMERA__

#include "Multithread/Component.hpp"

class Camera : public Component {
public:
    Matrix4 View;
    Matrix4 Projection;
    Matrix4 ProjectionView;

    void PreLoad() override;
    void Update() override;
};

#endif
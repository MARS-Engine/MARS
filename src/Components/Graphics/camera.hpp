#ifndef __CAMERA__
#define __CAMERA__

#include "Multithread/component.hpp"

enum camera_mode {
    PERSPECTIVE,
    ORTHO
};

class Camera : public component<Camera> {
private:
    camera_mode _camera_mode;
    vector4 _ortho_size;

    void update_camera();
public:
    matrix4 view;
    matrix4 perspective;
    matrix4 projection_view;
    matrix4 ortho;

    camera_mode get_mode();
    void set_mode(camera_mode _mode);

    vector4 get_ortho_size();
    void set_ortho_size(vector4 _size);

    void pre_load() override;
    void update() override;
};

#endif
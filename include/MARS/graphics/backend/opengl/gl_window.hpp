#ifndef MARS_GL_WINDOW_
#define MARS_GL_WINDOW_

#include <MARS/graphics/backend/template/window.hpp>

namespace mars_graphics {

    class gl_window : public window {
    public:
        void initialize(const std::string &_title, mars_math::vector2<int> _size) override;
    };
}

#endif
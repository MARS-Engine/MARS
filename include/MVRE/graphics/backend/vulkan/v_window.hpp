#ifndef __V__WINDOW__
#define __V__WINDOW__

#include <MVRE/graphics/backend/template/window.hpp>

namespace mvre_graphics {

    class v_window : public window {
    public:
        void initialize(const std::string &_title, mvre_math::vector2<int> _size) override;
    };
}

#endif
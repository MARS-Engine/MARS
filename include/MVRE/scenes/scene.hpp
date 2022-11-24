#ifndef __MVRE__SCENE__
#define __MVRE__SCENE__

#include <MVRE/graphics/graphics_instance.hpp>

namespace mvre_scenes {

    class scene {
    protected:
        mvre_graphics::graphics_instance* m_instance = nullptr;
    public:
        explicit scene(mvre_graphics::graphics_instance* _instance) { m_instance = _instance; }

        virtual void load() { }
    };
}

#endif
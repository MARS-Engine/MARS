#ifndef __SCENE__
#define __SCENE__

#include "MVRE/graphics/engine_instance.hpp"

namespace mvre_scenes {

    class scene {
    protected:
        mvre_graphics::engine_instance* m_instance = nullptr;
    public:
        explicit scene(mvre_graphics::engine_instance* _instance) { m_instance = _instance; }

        virtual void load() { }
    };
}

#endif
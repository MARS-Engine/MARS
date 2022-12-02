#ifndef __MVRE__SCENE__
#define __MVRE__SCENE__

#include <MVRE/graphics/graphics_instance.hpp>
#include <MVRE/engine/engine_handler.hpp>

namespace mvre_scenes {

    class scene {
    protected:
        mvre_graphics::graphics_instance* m_instance = nullptr;
        mvre_engine::engine_handler* m_engine = nullptr;
    public:
        explicit scene(mvre_graphics::graphics_instance* _instance, mvre_engine::engine_handler* _engine) { m_instance = _instance; m_engine = _engine; }

        virtual void load() { }
    };
}

#endif
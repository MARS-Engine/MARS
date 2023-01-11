#ifndef MARS_SCENE_
#define MARS_SCENE_

#include <MARS/graphics/graphics_instance.hpp>
#include <MARS/engine/engine_handler.hpp>

namespace mars_scenes {

    class scene {
    protected:
        mars_graphics::graphics_instance* m_instance = nullptr;
        mars_engine::engine_handler* m_engine = nullptr;
    public:
        explicit scene(mars_graphics::graphics_instance* _instance, mars_engine::engine_handler* _engine) { m_instance = _instance; m_engine = _engine; }

        virtual void load() { }
    };
}

#endif
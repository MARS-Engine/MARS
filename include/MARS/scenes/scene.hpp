#ifndef MARS_SCENE_
#define MARS_SCENE_

#include <MARS/graphics/graphics_engine.hpp>
#include <MARS/engine/object_engine.hpp>

namespace mars_scenes {

    class scene {
    protected:
        mars_ref<mars_graphics::graphics_engine> m_graphics;
        mars_ref<mars_engine::object_engine> m_engine;
    public:
        explicit scene(const mars_ref<mars_graphics::graphics_engine>& _instance, const mars_ref<mars_engine::object_engine>& _engine) { m_graphics = _instance; m_engine = _engine; }

        virtual void load() { }
    };
}

#endif
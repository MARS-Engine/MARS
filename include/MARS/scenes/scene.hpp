#ifndef MARS_SCENE_
#define MARS_SCENE_

#include <MARS/graphics/graphics_engine.hpp>
#include <MARS/engine/object_engine.hpp>

namespace mars_scenes {

    class scene {
    protected:
        std::shared_ptr<mars_graphics::graphics_engine> m_graphics;
        std::shared_ptr<mars_engine::object_engine> m_engine;
    public:
        explicit scene(const std::shared_ptr<mars_graphics::graphics_engine>& _instance, const std::shared_ptr<mars_engine::object_engine>& _engine) { m_graphics = _instance; m_engine = _engine; }

        virtual void load() { }
    };
}

#endif
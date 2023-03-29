#ifndef MARS_LIGHT_MANAGER_
#define MARS_LIGHT_MANAGER_

#include <MARS/debug/debug.hpp>
#include <MARS/math/vector4.hpp>
#include <MARS/loaders/wavefront_loader.hpp>
#include <MARS/graphics/graphics_engine.hpp>
#include <MARS/graphics/backend/template/shader_input.hpp>
#include <MARS/graphics/backend/template/buffer.hpp>
#include <MARS/graphics/attribute/vertex1.hpp>
#include <MARS/graphics/backend/template/shader_data.hpp>
#include <MARS/graphics/pipeline_manager.hpp>

#include <pl/safe_map.hpp>
#include <pl/safe_vector.hpp>

namespace mars_graphics {

    class shader;

    struct point_light {
        mars_math::vector4<float> Position;
        mars_math::vector3<float> Color;

        float Linear;
        float Quadratic;
        float Radius;
        float _Radius;
        float _Radius2;
    };

    struct scene_lights {
        point_light lights[32];
        int active_lights;
    };

    class light_manager {
    private:
        pl::safe_vector<point_light*> update_buffer;
        pl::safe_vector<point_light*> lights;

        std::shared_ptr<shader> light_shader;
        std::shared_ptr<pipeline> m_pipeline;
        std::shared_ptr<shader_data> m_data;
        std::shared_ptr<shader_input> m_input;
        std::shared_ptr<graphics_engine> m_graphics;

        scene_lights scene;
    public:

        inline void add_light(point_light* _light) {
            update_buffer.lock();
            update_buffer.push_back(_light);
            update_buffer.unlock();
        }


        void load(const std::shared_ptr<graphics_engine>& _graphics);
        void draw_lights();
        void destroy();
    };
}

#endif
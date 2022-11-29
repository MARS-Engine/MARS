#ifndef __MVRE__ENGINE__HANDLER__
#define __MVRE__ENGINE__HANDLER__

#include <pl/parallel.hpp>
#include <MVRE/graphics/graphics_instance.hpp>
#include <vector>

namespace mvre_engine {

    class engine_object;
    class engine_layer;

    class engine_handler {
    private:
        static int next_code;
        static pl::pl_job* m_job;
        static pl::safe_map<int, pl::safe_vector<engine_object*>> m_workers;
    public:
        static pl::safe_map<int, pl::safe_vector<engine_object*>>& workers() { return m_workers; };

        static void init();
        static void process_layer(engine_layer* _layer);
        static void clean();

        static engine_object* instance(engine_object* _obj, mvre_graphics::graphics_instance* _instance, engine_object* _parent);
        static engine_object* instance(engine_object* _obj, engine_object* _parent);
    };
}

#endif
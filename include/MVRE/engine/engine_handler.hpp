#ifndef __MVRE__ENGINE__HANDLER__
#define __MVRE__ENGINE__HANDLER__

#include "MVRE/graphics/engine_instance.hpp"
#include <vector>

namespace mvre_engine {

    class engine_worker;
    class engine_object;

    enum MVRE_EXECUTION_CODE {
        PRE_LOAD,
        LOAD,
        UPDATE,
        PRE_RENDER,
        RENDER,
        POST_RENDER,
        CLEAN
    };

    class engine_handler {
    private:
        static int next_code;
    public:
        static std::vector<engine_worker*> workers;

        static void init();
        static void execute(MVRE_EXECUTION_CODE _code);
        static void clean();

        static engine_object* instance(engine_object* _obj, mvre_graphics::engine_instance* _instance, engine_object* _parent);
        static engine_object* instance(engine_object* _obj, engine_object* _parent);
    };
}

#endif
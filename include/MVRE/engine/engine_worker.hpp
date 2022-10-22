#ifndef __MVRE__ENGINE__WORKER__
#define __MVRE__ENGINE__WORKER__

#include "pl/safe_vector.hpp"
#include "engine_object.hpp"
#include "engine_handler.hpp"

namespace mvre_engine {

    class engine_worker {
    public:
        pl::safe_vector<engine_object*> objects;

        void execute(MVRE_EXECUTION_CODE _code);
    };
}

#endif
#include "MVRE/engine/engine_worker.hpp"

using namespace mvre_engine;

void engine_worker::execute(mvre_engine::MVRE_EXECUTION_CODE _code) {
    for (auto obj : objects)
        obj->execute(_code);
}
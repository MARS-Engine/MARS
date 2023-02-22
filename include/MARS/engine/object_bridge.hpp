#ifndef MARS_OBJECT_BRIDGE_
#define MARS_OBJECT_BRIDGE_

#include "engine_object.hpp"

namespace mars_engine {

    class object_bridge {
    private:
        engine_object* m_object;
    public:
        [[nodiscard]] inline engine_object* get_engine_object() const { return m_object; }

        explicit object_bridge(engine_object* _object) { m_object = _object; }
    };
}

#endif
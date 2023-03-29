#ifndef MARS_OBJECT_BRIDGE_
#define MARS_OBJECT_BRIDGE_

#include "mars_object.hpp"

namespace mars_engine {

    class object_bridge {
    private:
        mars_object* m_object;
    public:
        [[nodiscard]] inline mars_object* get_engine_object() const { return m_object; }

        explicit object_bridge(mars_object* _object) { m_object = _object; }
    };
}

#endif
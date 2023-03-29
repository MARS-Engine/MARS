#include <MARS/engine/bridge.hpp>
#include <MARS/engine/mars_object.hpp>

mars_engine::bridge::bridge(mars_object& _object) {
    m_object = _object.get_ptr();
}
#include <MARS/engine/bridge.hpp>
#include <MARS/engine/mars_object.hpp>

mars_engine::bridge::bridge(const mars_object& _object) {
    m_object = _object;
}
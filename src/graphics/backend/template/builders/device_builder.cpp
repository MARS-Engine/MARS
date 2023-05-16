#include <MARS/graphics/backend/template/builders/device_builder.hpp>
#include <MARS/graphics/backend/template/device.hpp>

using namespace mars_graphics;

device_builder::device_builder(const std::shared_ptr<device>& _ptr) : graphics_builder<device>(_ptr) {
    m_available_devices = m_ref->find_devices();

    if (m_available_devices.empty())
        mars_debug::debug::error("MARS - Device Builder - Couldn't find a valid device");
}

std::shared_ptr<device> device_builder::build() {
    base_build();
    m_ref = m_available_devices[0];
    m_ref->create();
    return m_ref;
}
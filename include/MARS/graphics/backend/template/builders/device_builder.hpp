#ifndef MARS_DEVICE_BUILDER_
#define MARS_DEVICE_BUILDER_

#include "graphics_builder.hpp"
#include <vector>

namespace mars_graphics {
    class device;

    class device_builder : graphics_builder<device> {
    private:
        std::vector<std::shared_ptr<device>> m_available_devices;
    public:
        device_builder() = delete;

        explicit device_builder(const std::shared_ptr<device>& _ptr);

        std::shared_ptr<device> build();
    };
}

#endif
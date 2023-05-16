#ifndef MARS_DEVICE_
#define MARS_DEVICE_

#include "graphics_component.hpp"
#include "builders/device_builder.hpp"

namespace mars_graphics {
    class device : public graphics_component, public std::enable_shared_from_this<device> {
    private:
        virtual std::vector<std::shared_ptr<device>> find_devices() { return {}; }

        virtual void create() { }

        friend device_builder;
    public:
        using graphics_component::graphics_component;
    };
}

#endif
#pragma once

#include <mars/graphics/backend/descriptor.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {
    namespace graphics {

        inline descriptor descriptor_create(const device& _device, const descriptor_create_params& _params, size_t _frames_in_flight) {
            return _device.engine->get_impl<descriptor_impl>().descriptor_create(_device, _params, _frames_in_flight);
        }

        inline descriptor_set descriptor_set_create(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) {
            return _descriptor.engine->get_impl<descriptor_impl>().descriptor_set_create(_descriptor, _device, _pipeline, _params);
        }

        inline void descriptor_set_bind(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline& _pipeline, size_t _current_frame) {
            return _descriptor_set.engine->get_impl<descriptor_impl>().descriptor_set_bind(_descriptor_set, _command_buffer, _pipeline, _current_frame);
        }

        inline void descriptor_destroy(descriptor& _descriptor, const device& _device) {
            return _descriptor.engine->get_impl<descriptor_impl>().descriptor_destroy(_descriptor, _device);
        }
    } // namespace graphics
} // namespace mars
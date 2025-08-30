#pragma once

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {
    namespace graphics {

        inline command_pool command_pool_create(const device& _device) {
            return _device.engine->get_impl<command_pool_impl>().command_pool_create(_device);
        }

        inline std::vector<command_buffer> command_buffer_create(const command_pool& _command_pool, const device& _device, size_t _size) {
            return _device.engine->get_impl<command_pool_impl>().command_buffer_create(_command_pool, _device, _size);
        }

        inline void command_buffer_record(const command_buffer& _command_buffer) {
            return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_record(_command_buffer);
        }

        inline void command_buffer_record_end(const command_buffer& _command_buffer) {
            return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_record_end(_command_buffer);
        }

        inline void command_buffer_draw(const command_buffer& _command_buffer, const command_buffer_draw_params& _params) {
            return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_draw(_command_buffer, _params);
        }

        inline void command_pool_destroy(command_pool& _command_pool, const device& _device) {
            _command_pool.engine->get_impl<command_pool_impl>().command_pool_destroy(_command_pool, _device);
        }
    } // namespace graphics
} // namespace mars
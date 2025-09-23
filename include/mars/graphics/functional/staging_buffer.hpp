#pragma once

#include <mars/graphics/functional/buffer.hpp>
#include <mars/graphics/functional/command_pool.hpp>
#include <mars/graphics/functional/device.hpp>
#include <mars/graphics/functional/sync.hpp>

#include <cstring>
#include <utility>
#include <vector>

namespace mars {
    struct staging_buffer {
        command_pool pool;
        command_buffer staging_command;
        sync stanging_sync;
        buffer staging;
        void* data;
        size_t current_offset;
        std::vector<std::pair<buffer, size_t>> copy_list;
    };

    namespace graphics {
        inline staging_buffer staging_buffer_create(const device& _device, size_t _staging_size) {
            staging_buffer result;

            result.pool = command_pool_create(_device);
            result.staging_command = command_buffer_create(result.pool, _device, 1)[0];
            result.stanging_sync = sync_create(_device, 1);

            result.staging = buffer_create(_device, {
                                                        .buffer_type = MARS_BUFFER_TYPE_TRANSFER_SRC,
                                                        .buffer_property = MARS_BUFFER_PROPERTY_HOST_VISIBLE,
                                                        .allocated_size = _staging_size,
                                                    });

            result.data = buffer_map(result.staging, _device, _staging_size, 0);
            result.current_offset = 0;
            return result;
        }

        inline void staging_buffer_copy(staging_buffer& _stanging, const void* _data, const buffer& _dst) {
            std::memcpy((char*)_stanging.data + _stanging.current_offset, _data, _dst.allocated_size);
            _stanging.copy_list.emplace_back(_dst, _stanging.current_offset);
            _stanging.current_offset += _dst.allocated_size;
        }

        inline void staging_buffer_commit(staging_buffer& _stanging, const device& _device) {
            sync_wait(_stanging.stanging_sync, _device, 0);
            sync_reset(_stanging.stanging_sync, _device, 0);

            command_buffer_record(_stanging.staging_command);

            for (std::pair<buffer, size_t>& entry : _stanging.copy_list)
                buffer_copy(entry.first, _stanging.staging, _stanging.staging_command, entry.second);

            command_buffer_record_end(_stanging.staging_command);
            _stanging.current_offset = 0;

            device_submit_graphics_queue(_device, _stanging.stanging_sync, {}, &_stanging.staging_command, 1);
        }

        inline void staging_buffer_destroy(staging_buffer& _buffer, const device& _device) {
            buffer_unmap(_buffer.staging, _device);
            buffer_destroy(_buffer.staging, _device);
            command_pool_destroy(_buffer.pool, _device);
            sync_destroy(_buffer.stanging_sync, _device);
            _buffer = {};
        }
    } // namespace graphics
} // namespace mars
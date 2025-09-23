#pragma once

#include <mars/graphics/functional/buffer.hpp>
#include <vector>

namespace mars {

    struct dynamic_buffer {
        std::vector<std::pair<buffer, void*>> buffers;
    };

    namespace graphics {
        inline dynamic_buffer dynamic_buffer_create(const device& _device, mars::buffer_create_params _params, size_t _size) {
            dynamic_buffer result;
            result.buffers.reserve(_size);

            for (size_t i = 0; i < _size; i++) {
                buffer buffer = buffer_create(_device, _params);
                void* data = buffer_map(buffer, _device, _params.allocated_size, 0);
                result.buffers.emplace_back(buffer, data);
            }

            return result;
        }

        inline void* dynamic_buffer_frame_buffer(const dynamic_buffer& _buffer, size_t _curent_frame) {
            if (_curent_frame > _buffer.buffers.size())
                return nullptr;

            return _buffer.buffers[_curent_frame].second;
        }

        inline void dynamic_buffer_destroy(dynamic_buffer& _buffer, const device& _device) {
            for (auto& pair : _buffer.buffers) {
                buffer_unmap(pair.first, _device);
                buffer_destroy(pair.first, _device);
            }

            _buffer = {};
        }
    } // namespace graphics
} // namespace mars
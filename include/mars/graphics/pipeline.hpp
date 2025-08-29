#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>
#include <mars/graphics/backend/pipeline.hpp>

namespace mars {
    namespace graphics {

        inline pipeline pipeline_create(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup) {
            return _device.engine->get_impl<pipeline_impl>().pipeline_create(_device, _render_pass, _setup);
        }

        inline void pipeline_destroy(pipeline& _pipeline, const device& _device) {
            _pipeline.engine->get_impl<pipeline_impl>().pipeline_destroy(_pipeline, _device);
        }
    } // namespace graphics
} // namespace mars
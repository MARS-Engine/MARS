#pragma once

#include <mars/graphics/backend/descriptor.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars::graphics {

inline mars::descriptor descriptor_create(const mars::device& device, const mars::descriptor_create_params& params, size_t frames_in_flight) {
	return device.engine->get_impl<descriptor_impl>().descriptor_create(device, params, frames_in_flight);
}

inline mars::descriptor_set descriptor_set_create(const mars::descriptor& descriptor, const mars::device& device, const mars::pipeline& pipeline, const std::vector<mars::descriptor_set_create_params>& params) {
	return descriptor.engine->get_impl<descriptor_impl>().descriptor_set_create(descriptor, device, pipeline, params);
}

inline mars::descriptor_set descriptor_set_create_compute(const mars::descriptor& descriptor, const mars::device& device, const mars::compute_pipeline& pipeline, const std::vector<mars::descriptor_set_create_params>& params) {
	return descriptor.engine->get_impl<descriptor_impl>().descriptor_set_create_compute(descriptor, device, pipeline, params);
}

inline void descriptor_set_bind(const mars::descriptor_set& descriptor_set, const mars::command_buffer& command_buffer, const mars::pipeline& pipeline, size_t current_frame) {
	return descriptor_set.engine->get_impl<descriptor_impl>().descriptor_set_bind(descriptor_set, command_buffer, pipeline, current_frame);
}

inline void descriptor_set_bind_compute(const mars::descriptor_set& descriptor_set, const mars::command_buffer& command_buffer, const mars::compute_pipeline& pipeline, size_t current_frame) {
	return descriptor_set.engine->get_impl<descriptor_impl>().descriptor_set_bind_compute(descriptor_set, command_buffer, pipeline, current_frame);
}

inline void descriptor_set_update_cbv(mars::descriptor_set& descriptor_set, size_t binding, const mars::buffer& buffer) {
	return descriptor_set.engine->get_impl<descriptor_impl>().descriptor_set_update_cbv(descriptor_set, binding, buffer);
}

inline void descriptor_destroy(mars::descriptor& descriptor, const mars::device& device) {
	return descriptor.engine->get_impl<descriptor_impl>().descriptor_destroy(descriptor, device);
}

} // namespace mars::graphics

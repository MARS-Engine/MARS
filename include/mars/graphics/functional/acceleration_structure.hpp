#pragma once

#include <mars/graphics/backend/acceleration_structure.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars::graphics {

inline blas blas_create(const device& _device, const command_buffer& _command_buffer, const blas_create_params& _params) {
	return _device.engine->get_impl<acceleration_structure_impl>().blas_create(_device, _command_buffer, _params);
}

inline void blas_update(blas& _blas, const command_buffer& _command_buffer, const blas_create_params& _params) {
	return _blas.engine->get_impl<acceleration_structure_impl>().blas_update(_blas, _command_buffer, _params);
}

inline void blas_destroy(blas& _blas, const device& _device) {
	return _blas.engine->get_impl<acceleration_structure_impl>().blas_destroy(_blas, _device);
}

inline tlas tlas_create(const device& _device, const tlas_create_params& _params) {
	return _device.engine->get_impl<acceleration_structure_impl>().tlas_create(_device, _params);
}

inline void tlas_build(tlas& _tlas, const command_buffer& _command_buffer, const acceleration_structure_instance* _instances, uint32_t _count) {
	return _tlas.engine->get_impl<acceleration_structure_impl>().tlas_build(_tlas, _command_buffer, _instances, _count);
}

inline uint32_t tlas_get_srv_index(const tlas& _tlas) {
	return _tlas.engine->get_impl<acceleration_structure_impl>().tlas_get_srv_index(_tlas);
}

inline void tlas_destroy(tlas& _tlas, const device& _device) {
	return _tlas.engine->get_impl<acceleration_structure_impl>().tlas_destroy(_tlas, _device);
}

} // namespace mars::graphics

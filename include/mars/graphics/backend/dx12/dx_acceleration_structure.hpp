#pragma once

#include <mars/graphics/backend/acceleration_structure.hpp>

namespace mars::graphics::dx {
struct dx_acceleration_structure_impl {
	static blas dx_blas_create(const device& _device, const command_buffer& _command_buffer, const blas_create_params& _params);
	static void dx_blas_update(blas& _blas, const command_buffer& _command_buffer, const blas_create_params& _params);
	static void dx_blas_destroy(blas& _blas, const device& _device);

	static tlas dx_tlas_create(const device& _device, const tlas_create_params& _params);
	static void dx_tlas_build(tlas& _tlas, const command_buffer& _command_buffer, const acceleration_structure_instance* _instances, uint32_t _count);
	static uint32_t dx_tlas_get_srv_index(const tlas& _tlas);
	static void dx_tlas_destroy(tlas& _tlas, const device& _device);
};
} // namespace mars::graphics::dx

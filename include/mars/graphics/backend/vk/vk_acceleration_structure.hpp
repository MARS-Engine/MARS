#pragma once

#include <mars/graphics/backend/acceleration_structure.hpp>

namespace mars::graphics::vk {
struct vk_acceleration_structure_impl {
	static blas vk_blas_create(const device& _device, const command_buffer& _command_buffer, const blas_create_params& _params);
	static void vk_blas_update(blas& _blas, const command_buffer& _command_buffer, const blas_create_params& _params);
	static void vk_blas_destroy(blas& _blas, const device& _device);

	static tlas vk_tlas_create(const device& _device, const tlas_create_params& _params);
	static void vk_tlas_build(tlas& _tlas, const command_buffer& _command_buffer, const acceleration_structure_instance* _instances, uint32_t _count);
	static uint32_t vk_tlas_get_srv_index(const tlas& _tlas);
	static void vk_tlas_destroy(tlas& _tlas, const device& _device);
};
} // namespace mars::graphics::vk

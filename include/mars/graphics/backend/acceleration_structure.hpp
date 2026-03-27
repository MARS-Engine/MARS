#pragma once

#include <mars/meta/type_erased.hpp>
#include <mars/math/row_matrix3x4.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mars {
struct device;
struct command_buffer;
struct graphics_backend_functions;

struct blas {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
	size_t size = 0;
};

struct tlas {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
	size_t size = 0;
};

struct blas_geometry {
	meta::type_erased_ptr vertex_buffer_handle;
	meta::type_erased_ptr index_buffer_handle;
	uint32_t vertex_count = 0;
	uint32_t vertex_stride = 0;
	uint32_t index_count = 0;
};

struct blas_create_params {
	std::vector<blas_geometry> geometries;
	bool allow_update = false;
};

struct acceleration_structure_instance {
	row_matrix3x4<float> transform;
	uint32_t instance_id = 0;       // 24-bit custom index visible to shaders (instanceCustomIndex)
	uint8_t mask = 0;               // visibility mask
	uint32_t sbt_record_offset = 0; // hit group SBT record offset
	const blas* bottom_level = nullptr;   // associated BLAS
};

struct tlas_create_params {
	uint32_t max_instance_count = 0;
	bool allow_update = false;
};

struct acceleration_structure_impl {
	blas (*blas_create)(const device& _device, const command_buffer& _command_buffer, const blas_create_params& _params) = nullptr;
	void (*blas_update)(blas& _blas, const command_buffer& _command_buffer, const blas_create_params& _params) = nullptr;
	void (*blas_destroy)(blas& _blas, const device& _device) = nullptr;

	tlas (*tlas_create)(const device& _device, const tlas_create_params& _params) = nullptr;
	void (*tlas_build)(tlas& _tlas, const command_buffer& _command_buffer, const acceleration_structure_instance* _instances, uint32_t _count) = nullptr;
	uint32_t (*tlas_get_srv_index)(const tlas& _tlas) = nullptr;
	void (*tlas_destroy)(tlas& _tlas, const device& _device) = nullptr;
};
} // namespace mars

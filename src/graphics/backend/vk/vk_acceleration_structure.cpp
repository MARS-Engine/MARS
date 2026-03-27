#include <mars/graphics/backend/vk/vk_acceleration_structure.hpp>
#include <mars/graphics/functional/buffer.hpp>

#include "vk_internal.hpp"

namespace mars::graphics::vk {
namespace {
buffer create_acceleration_structure_buffer(const device& _device, size_t _size, uint32_t _buffer_type, uint32_t _buffer_property) {
	buffer_create_params create_params = {};
	create_params.buffer_type = _buffer_type;
	create_params.buffer_property = _buffer_property;
	create_params.allocated_size = _size;
	return buffer_create(_device, create_params);
}

} // namespace

blas vk_acceleration_structure_impl::vk_blas_create(const device& _device, const command_buffer& _command_buffer, const blas_create_params& _params) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();

	// Build VkAccelerationStructureGeometryKHR entries from each blas_geometry
	std::vector<VkAccelerationStructureGeometryKHR> geometries;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> build_ranges;
	geometries.reserve(_params.geometries.size());
	build_ranges.reserve(_params.geometries.size());

	for (const auto& geo : _params.geometries) {
		auto* vertex_buffer_data = geo.vertex_buffer_handle.expect<vk_buffer_data>();
		auto* index_buffer_data = geo.index_buffer_handle.expect<vk_buffer_data>();

		vk_cmd_pipeline_buffer_barrier2(
			command_buffer_data->command_buffer,
			vertex_buffer_data->current_stage,
			vertex_buffer_data->current_access,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
			vertex_buffer_data->buffer,
			0u,
			vertex_buffer_data->size
		);
		vertex_buffer_data->current_stage = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		vertex_buffer_data->current_access = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

		vk_cmd_pipeline_buffer_barrier2(
			command_buffer_data->command_buffer,
			index_buffer_data->current_stage,
			index_buffer_data->current_access,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
			index_buffer_data->buffer,
			0u,
			index_buffer_data->size
		);
		index_buffer_data->current_stage = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		index_buffer_data->current_access = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

		VkAccelerationStructureGeometryTrianglesDataKHR triangles = {};
		triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData.deviceAddress = vertex_buffer_data->device_address;
		triangles.vertexStride = geo.vertex_stride;
		triangles.maxVertex = geo.vertex_count - 1u;
		triangles.indexType = VK_INDEX_TYPE_UINT32;
		triangles.indexData.deviceAddress = index_buffer_data->device_address;

		VkAccelerationStructureGeometryKHR geometry = {};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.geometry.triangles = triangles;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		geometries.push_back(geometry);

		VkAccelerationStructureBuildRangeInfoKHR range = {};
		range.primitiveCount = geo.index_count / 3u;
		build_ranges.push_back(range);
	}

	const auto build_flags = _params.allow_update ? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

	VkAccelerationStructureBuildGeometryInfoKHR build_info = {};
	build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	build_info.flags = build_flags;
	build_info.geometryCount = static_cast<uint32_t>(geometries.size());
	build_info.pGeometries = geometries.data();

	// Gather primitive counts for size query
	std::vector<uint32_t> prim_counts;
	prim_counts.reserve(build_ranges.size());
	for (const auto& r : build_ranges)
		prim_counts.push_back(r.primitiveCount);

	VkAccelerationStructureBuildSizesInfoKHR sizes = {};
	sizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	device_data->get_acceleration_structure_build_sizes(device_data->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, prim_counts.data(), &sizes);

	auto* blas_data = new vk_blas_data();
	blas_data->size = sizes.accelerationStructureSize;

	blas_data->result_buffer = create_acceleration_structure_buffer(
		_device,
		sizes.accelerationStructureSize,
		MARS_BUFFER_TYPE_ACCELERATION_STRUCTURE_STORAGE,
		MARS_BUFFER_PROPERTY_DEVICE_LOCAL
	);
	blas_data->scratch_buffer = create_acceleration_structure_buffer(
		_device,
		sizes.buildScratchSize,
		MARS_BUFFER_TYPE_UNORDERED_ACCESS,
		MARS_BUFFER_PROPERTY_DEVICE_LOCAL
	);

	if (blas_data->result_buffer.engine == nullptr || blas_data->scratch_buffer.engine == nullptr) {
		if (blas_data->scratch_buffer.engine != nullptr)
			buffer_destroy(blas_data->scratch_buffer, _device);
		if (blas_data->result_buffer.engine != nullptr)
			buffer_destroy(blas_data->result_buffer, _device);
		delete blas_data;
		return {};
	}

	auto* result_buffer_data = blas_data->result_buffer.data.expect<vk_buffer_data>();
	auto* scratch_buffer_data = blas_data->scratch_buffer.data.expect<vk_buffer_data>();

	// Create the AS object
	VkAccelerationStructureCreateInfoKHR as_ci = {};
	as_ci.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	as_ci.buffer = result_buffer_data->buffer;
	as_ci.size = sizes.accelerationStructureSize;
	as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	device_data->create_acceleration_structure(device_data->device, &as_ci, nullptr, &blas_data->acceleration_structure);

	// Retrieve device address for TLAS instance records
	VkAccelerationStructureDeviceAddressInfoKHR addr_info = {};
	addr_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addr_info.accelerationStructure = blas_data->acceleration_structure;
	blas_data->device_address = device_data->get_acceleration_structure_device_address(device_data->device, &addr_info);

	// Get scratch device address
	const VkDeviceAddress scratch_addr = scratch_buffer_data->device_address;

	// Record the build command
	build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	build_info.dstAccelerationStructure = blas_data->acceleration_structure;
	build_info.scratchData.deviceAddress = scratch_addr;

	const VkAccelerationStructureBuildRangeInfoKHR* range_ptr = build_ranges.data();
	device_data->cmd_build_acceleration_structures(command_buffer_data->command_buffer, 1u, &build_info, &range_ptr);
	vk_cmd_pipeline_memory_barrier2(command_buffer_data->command_buffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_SHADER_READ_BIT);

	blas result = {};
	result.engine = _device.engine;
	result.data.store(blas_data);
	result.size = sizes.accelerationStructureSize;
	return result;
}

void vk_acceleration_structure_impl::vk_blas_update(blas& _blas, const command_buffer& _command_buffer, const blas_create_params& _params) {
	auto* blas_data = _blas.data.expect<vk_blas_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* device_data = command_buffer_data->device_data;
	auto* scratch_buffer_data = blas_data->scratch_buffer.data.expect<vk_buffer_data>();

	std::vector<VkAccelerationStructureGeometryKHR> geometries;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> build_ranges;
	geometries.reserve(_params.geometries.size());
	build_ranges.reserve(_params.geometries.size());

	for (const auto& geo : _params.geometries) {
		auto* vertex_buffer_data = geo.vertex_buffer_handle.expect<vk_buffer_data>();
		auto* index_buffer_data = geo.index_buffer_handle.expect<vk_buffer_data>();

		vk_cmd_pipeline_buffer_barrier2(
			command_buffer_data->command_buffer,
			vertex_buffer_data->current_stage,
			vertex_buffer_data->current_access,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
			vertex_buffer_data->buffer,
			0u,
			vertex_buffer_data->size
		);
		vertex_buffer_data->current_stage = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		vertex_buffer_data->current_access = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

		vk_cmd_pipeline_buffer_barrier2(
			command_buffer_data->command_buffer,
			index_buffer_data->current_stage,
			index_buffer_data->current_access,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
			index_buffer_data->buffer,
			0u,
			index_buffer_data->size
		);
		index_buffer_data->current_stage = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		index_buffer_data->current_access = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

		VkAccelerationStructureGeometryTrianglesDataKHR triangles = {};
		triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData.deviceAddress = vertex_buffer_data->device_address;
		triangles.vertexStride = geo.vertex_stride;
		triangles.maxVertex = geo.vertex_count - 1u;
		triangles.indexType = VK_INDEX_TYPE_UINT32;
		triangles.indexData.deviceAddress = index_buffer_data->device_address;

		VkAccelerationStructureGeometryKHR geometry = {};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.geometry.triangles = triangles;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		geometries.push_back(geometry);

		VkAccelerationStructureBuildRangeInfoKHR range = {};
		range.primitiveCount = geo.index_count / 3u;
		build_ranges.push_back(range);
	}

	const VkDeviceAddress scratch_addr = scratch_buffer_data->device_address;

	VkAccelerationStructureBuildGeometryInfoKHR build_info = {};
	build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	build_info.srcAccelerationStructure = blas_data->acceleration_structure;
	build_info.dstAccelerationStructure = blas_data->acceleration_structure;
	build_info.geometryCount = static_cast<uint32_t>(geometries.size());
	build_info.pGeometries = geometries.data();
	build_info.scratchData.deviceAddress = scratch_addr;

	const VkAccelerationStructureBuildRangeInfoKHR* range_ptr = build_ranges.data();
	device_data->cmd_build_acceleration_structures(command_buffer_data->command_buffer, 1u, &build_info, &range_ptr);
	vk_cmd_pipeline_memory_barrier2(
		command_buffer_data->command_buffer,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_SHADER_READ_BIT
	);
}

void vk_acceleration_structure_impl::vk_blas_destroy(blas& _blas, const device& _device) {
	auto* blas_data = _blas.data.expect<vk_blas_data>();
	auto* device_data = _device.data.expect<vk_device_data>();

	device_data->destroy_acceleration_structure(device_data->device, blas_data->acceleration_structure, nullptr);
	buffer_destroy(blas_data->scratch_buffer, _device);
	buffer_destroy(blas_data->result_buffer, _device);
	delete blas_data;
	_blas = {};
}

tlas vk_acceleration_structure_impl::vk_tlas_create(const device& _device, const tlas_create_params& _params) {
	auto* device_data = _device.data.expect<vk_device_data>();

	// Dummy size query using max instance count
	VkAccelerationStructureGeometryInstancesDataKHR instances_data = {};
	instances_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;

	VkAccelerationStructureGeometryKHR geom = {};
	geom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geom.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geom.geometry.instances = instances_data;

	const auto build_flags = _params.allow_update
		? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR
		: VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

	VkAccelerationStructureBuildGeometryInfoKHR build_info = {};
	build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	build_info.flags = build_flags;
	build_info.geometryCount = 1u;
	build_info.pGeometries = &geom;

	VkAccelerationStructureBuildSizesInfoKHR sizes = {};
	sizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	device_data->get_acceleration_structure_build_sizes(
		device_data->device,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&build_info,
		&_params.max_instance_count,
		&sizes
	);

	auto* tlas_data = new vk_tlas_data();
	tlas_data->size = sizes.accelerationStructureSize;
	tlas_data->max_instance_count = _params.max_instance_count;
	tlas_data->allow_update = _params.allow_update;

	tlas_data->result_buffer = create_acceleration_structure_buffer(
		_device,
		sizes.accelerationStructureSize,
		MARS_BUFFER_TYPE_ACCELERATION_STRUCTURE_STORAGE,
		MARS_BUFFER_PROPERTY_DEVICE_LOCAL
	);
	tlas_data->scratch_buffer = create_acceleration_structure_buffer(
		_device,
		sizes.buildScratchSize,
		MARS_BUFFER_TYPE_UNORDERED_ACCESS,
		MARS_BUFFER_PROPERTY_DEVICE_LOCAL
	);
	tlas_data->instance_buffer = create_acceleration_structure_buffer(
		_device,
		_params.max_instance_count * sizeof(VkAccelerationStructureInstanceKHR),
		MARS_BUFFER_TYPE_ACCELERATION_STRUCTURE_BUILD_INPUT,
		MARS_BUFFER_PROPERTY_HOST_VISIBLE
	);
	if (tlas_data->result_buffer.engine == nullptr || tlas_data->scratch_buffer.engine == nullptr || tlas_data->instance_buffer.engine == nullptr) {
		if (tlas_data->instance_buffer.engine)
			buffer_destroy(tlas_data->instance_buffer, _device);
		if (tlas_data->scratch_buffer.engine)
			buffer_destroy(tlas_data->scratch_buffer, _device);
		if (tlas_data->result_buffer.engine)
			buffer_destroy(tlas_data->result_buffer, _device);
		delete tlas_data;
		return {};
	}

	auto* result_buffer_data = tlas_data->result_buffer.data.expect<vk_buffer_data>();
	tlas_data->instance_mapped = buffer_map(tlas_data->instance_buffer, _device, 0u, 0u);

	// Create the AS object
	VkAccelerationStructureCreateInfoKHR as_ci = {};
	as_ci.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	as_ci.buffer = result_buffer_data->buffer;
	as_ci.size = sizes.accelerationStructureSize;
	as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	device_data->create_acceleration_structure(device_data->device, &as_ci, nullptr, &tlas_data->acceleration_structure);

	// Allocate a bindless descriptor slot for the TLAS
	tlas_data->srv_bindless_idx = vk_allocate_bindless_srv_slot(device_data);

	VkWriteDescriptorSetAccelerationStructureKHR as_write = {};
	as_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	as_write.accelerationStructureCount = 1u;
	as_write.pAccelerationStructures = &tlas_data->acceleration_structure;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = &as_write;
	write.dstSet = device_data->bindless_set;
	write.dstBinding = 0u; // SRV binding
	write.dstArrayElement = tlas_data->srv_bindless_idx;
	write.descriptorCount = 1u;
	write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	vkUpdateDescriptorSets(device_data->device, 1u, &write, 0u, nullptr);

	tlas result = {};
	result.engine = _device.engine;
	result.data.store(tlas_data);
	result.size = sizes.accelerationStructureSize;
	return result;
}

void vk_acceleration_structure_impl::vk_tlas_build(tlas& _tlas, const command_buffer& _command_buffer, const acceleration_structure_instance* _instances, uint32_t _count) {
	auto* tlas_data = _tlas.data.expect<vk_tlas_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* device_data = command_buffer_data->device_data;
	auto* instance_buffer_data = tlas_data->instance_buffer.data.expect<vk_buffer_data>();
	auto* scratch_buffer_data = tlas_data->scratch_buffer.data.expect<vk_buffer_data>();

	// Copy instance data into the persistently-mapped host buffer
	auto* dst = static_cast<VkAccelerationStructureInstanceKHR*>(tlas_data->instance_mapped);
	for (uint32_t i = 0u; i < _count; ++i) {
		const auto& src = _instances[i];
		VkAccelerationStructureInstanceKHR& inst = dst[i];
		static_assert(sizeof(VkTransformMatrixKHR) == sizeof(row_matrix3x4<float>));
		std::memcpy(&inst.transform, &src.transform, sizeof(VkTransformMatrixKHR));
		inst.instanceCustomIndex = src.instance_id & 0x00FFFFFFu;
		inst.mask = src.mask;
		inst.instanceShaderBindingTableRecordOffset = src.sbt_record_offset;
		inst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		auto* blas_data = src.bottom_level->data.expect<vk_blas_data>();
		inst.accelerationStructureReference = blas_data->device_address;
	}

	const VkDeviceAddress inst_addr = instance_buffer_data->device_address;
	const VkDeviceAddress scratch_addr = scratch_buffer_data->device_address;

	// Ensure instance writes are visible before AS build reads them
	vk_cmd_pipeline_buffer_barrier2(
		command_buffer_data->command_buffer,
		VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
		instance_buffer_data->buffer, 0u, _count * sizeof(VkAccelerationStructureInstanceKHR)
	);

	VkAccelerationStructureGeometryInstancesDataKHR instances_data = {};
	instances_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	instances_data.data.deviceAddress = inst_addr;

	VkAccelerationStructureGeometryKHR geom = {};
	geom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geom.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geom.geometry.instances = instances_data;

	VkAccelerationStructureBuildGeometryInfoKHR build_info = {};
	build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	if (tlas_data->allow_update) {
		build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
		build_info.mode = tlas_data->has_been_built
			? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR
			: VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		if (tlas_data->has_been_built)
			build_info.srcAccelerationStructure = tlas_data->acceleration_structure;
	}
	else {
		build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	}
	build_info.dstAccelerationStructure = tlas_data->acceleration_structure;
	build_info.geometryCount = 1u;
	build_info.pGeometries = &geom;
	build_info.scratchData.deviceAddress = scratch_addr;

	VkAccelerationStructureBuildRangeInfoKHR range = {};
	range.primitiveCount = _count;
	const VkAccelerationStructureBuildRangeInfoKHR* range_ptr = &range;
	device_data->cmd_build_acceleration_structures(command_buffer_data->command_buffer, 1u, &build_info, &range_ptr);
	vk_cmd_pipeline_memory_barrier2(
		command_buffer_data->command_buffer,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_SHADER_READ_BIT
	);
	tlas_data->has_been_built = true;
}

uint32_t vk_acceleration_structure_impl::vk_tlas_get_srv_index(const tlas& _tlas) {
	return _tlas.data.expect<vk_tlas_data>()->srv_bindless_idx;
}

void vk_acceleration_structure_impl::vk_tlas_destroy(tlas& _tlas, const device& _device) {
	auto* tlas_data = _tlas.data.expect<vk_tlas_data>();
	auto* device_data = _device.data.expect<vk_device_data>();

	vk_release_bindless_srv_slot(device_data, tlas_data->srv_bindless_idx);
	device_data->destroy_acceleration_structure(device_data->device, tlas_data->acceleration_structure, nullptr);
	buffer_unmap(tlas_data->instance_buffer, _device);
	buffer_destroy(tlas_data->scratch_buffer, _device);
	buffer_destroy(tlas_data->instance_buffer, _device);
	buffer_destroy(tlas_data->result_buffer, _device);
	delete tlas_data;
	_tlas = {};
}

} // namespace mars::graphics::vk

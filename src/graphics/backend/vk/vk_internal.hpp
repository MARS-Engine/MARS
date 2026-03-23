#pragma once

#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/append_buffer.hpp>
#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/command_queue.hpp>
#include <mars/graphics/backend/descriptor.hpp>
#include <mars/graphics/backend/indirect_executor.hpp>
#include <mars/graphics/backend/pipeline.hpp>
#include <mars/graphics/backend/render_pass.hpp>
#include <mars/graphics/backend/texture.hpp>
#include <mars/meta.hpp>
#include <mars/meta/type_erased.hpp>

#include <vulkan/vulkan.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <deque>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mars::graphics::vk {

inline mars::log_channel& vk_log_channel() {
	static mars::log_channel channel("vk");
	return channel;
}

template <auto Function, typename... Args>
inline VkResult vk_expect(Args&&... args) {
	return mars::logger::log_expect<Function>(vk_log_channel(), [](VkResult result) { return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR; }, std::forward<Args>(args)...);
}

template <auto FunctionMember, typename Object, typename... Args>
	requires(std::is_member_object_pointer_v<decltype(FunctionMember)>)
inline VkResult vk_expect(Object&& object, Args&&... args) {
	return mars::logger::log_expect<FunctionMember>(vk_log_channel(), std::forward<Object>(object), [](VkResult result) { return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR; }, std::forward<Args>(args)...);
}

template <typename Function, typename... Args>
inline VkResult vk_expect(Function&& function, Args&&... args) {
	return mars::logger::log_expect(vk_log_channel(), std::forward<Function>(function), [](VkResult result) { return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR; }, std::forward<Args>(args)...);
}

inline VkPipelineStageFlags2 vk_stage_flags2(VkPipelineStageFlags flags) {
	return static_cast<VkPipelineStageFlags2>(flags);
}

inline VkAccessFlags2 vk_access_flags2(VkAccessFlags flags) {
	return static_cast<VkAccessFlags2>(flags);
}

inline void vk_cmd_pipeline_memory_barrier2(VkCommandBuffer command_buffer, VkPipelineStageFlags src_stage, VkAccessFlags src_access, VkPipelineStageFlags dst_stage, VkAccessFlags dst_access, VkDependencyFlags dependency_flags = 0u) {
	VkMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
	barrier.srcStageMask = vk_stage_flags2(src_stage);
	barrier.srcAccessMask = vk_access_flags2(src_access);
	barrier.dstStageMask = vk_stage_flags2(dst_stage);
	barrier.dstAccessMask = vk_access_flags2(dst_access);

	VkDependencyInfo dependency_info = {};
	dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency_info.dependencyFlags = dependency_flags;
	dependency_info.memoryBarrierCount = 1u;
	dependency_info.pMemoryBarriers = &barrier;
	vkCmdPipelineBarrier2(command_buffer, &dependency_info);
}

inline void vk_cmd_pipeline_buffer_barrier2(
	VkCommandBuffer command_buffer,
	VkPipelineStageFlags src_stage,
	VkAccessFlags src_access,
	VkPipelineStageFlags dst_stage,
	VkAccessFlags dst_access,
	VkBuffer buffer,
	VkDeviceSize offset,
	VkDeviceSize size,
	VkDependencyFlags dependency_flags = 0u
) {
	VkBufferMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
	barrier.srcStageMask = vk_stage_flags2(src_stage);
	barrier.srcAccessMask = vk_access_flags2(src_access);
	barrier.dstStageMask = vk_stage_flags2(dst_stage);
	barrier.dstAccessMask = vk_access_flags2(dst_access);
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = buffer;
	barrier.offset = offset;
	barrier.size = size;

	VkDependencyInfo dependency_info = {};
	dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency_info.dependencyFlags = dependency_flags;
	dependency_info.bufferMemoryBarrierCount = 1u;
	dependency_info.pBufferMemoryBarriers = &barrier;
	vkCmdPipelineBarrier2(command_buffer, &dependency_info);
}

inline void vk_cmd_pipeline_image_barrier2(
	VkCommandBuffer command_buffer,
	VkPipelineStageFlags src_stage,
	VkAccessFlags src_access,
	VkPipelineStageFlags dst_stage,
	VkAccessFlags dst_access,
	VkImageLayout old_layout,
	VkImageLayout new_layout,
	VkImage image,
	const VkImageSubresourceRange& subresource_range,
	VkDependencyFlags dependency_flags = 0u
) {
	VkImageMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.srcStageMask = vk_stage_flags2(src_stage);
	barrier.srcAccessMask = vk_access_flags2(src_access);
	barrier.dstStageMask = vk_stage_flags2(dst_stage);
	barrier.dstAccessMask = vk_access_flags2(dst_access);
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = subresource_range;

	VkDependencyInfo dependency_info = {};
	dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency_info.dependencyFlags = dependency_flags;
	dependency_info.imageMemoryBarrierCount = 1u;
	dependency_info.pImageMemoryBarriers = &barrier;
	vkCmdPipelineBarrier2(command_buffer, &dependency_info);
}

inline size_t vk_align_up(size_t value, size_t alignment) {
	if (alignment == 0)
		return value;
	return (value + alignment - 1u) & ~(alignment - 1u);
}

struct vk_queue_data {
	VkQueue queue = VK_NULL_HANDLE;
	uint32_t family_index = std::numeric_limits<uint32_t>::max();
};

enum class vk_sampler_kind : uint8_t {
	none = 0,
	linear_clamp = 1,
	point_clamp = 2,
};

struct vk_swapchain_data;
struct vk_framebuffer_data;

struct vk_device_data {
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties physical_device_properties = {};
	VkPhysicalDeviceMemoryProperties memory_properties = {};
	VkDevice device = VK_NULL_HANDLE;

	vk_queue_data direct_queue = {};
	vk_queue_data compute_queue = {};
	vk_queue_data copy_queue = {};

	static constexpr uint32_t BINDLESS_SRV_BASE = 0u;
	static constexpr uint32_t BINDLESS_SRV_COUNT = 32768u;
	static constexpr uint32_t BINDLESS_UAV_BASE = 32768u;
	static constexpr uint32_t BINDLESS_UAV_COUNT = 32768u;
	static constexpr uint32_t BINDLESS_TOTAL = 65536u;

	VkDescriptorPool bindless_pool = VK_NULL_HANDLE;
	VkDescriptorSetLayout bindless_set_layout = VK_NULL_HANDLE;
	VkDescriptorSet bindless_set = VK_NULL_HANDLE;
	uint32_t next_bindless_srv_idx = BINDLESS_SRV_BASE;
	uint32_t next_bindless_uav_idx = BINDLESS_UAV_BASE;
	std::vector<std::pair<uint32_t, uint32_t>> free_bindless_srv_ranges;
	std::vector<std::pair<uint32_t, uint32_t>> free_bindless_uav_ranges;

	VkDescriptorPool empty_descriptor_pool = VK_NULL_HANDLE;
	VkDescriptorSetLayout empty_set_layout = VK_NULL_HANDLE;
	VkDescriptorSet empty_set = VK_NULL_HANDLE;

	VkDescriptorPool sampler_descriptor_pool = VK_NULL_HANDLE;
	VkSampler linear_clamp_sampler = VK_NULL_HANDLE;
	VkSampler point_clamp_sampler = VK_NULL_HANDLE;

	PFN_vkCmdBeginDebugUtilsLabelEXT cmd_begin_debug_label = nullptr;
	PFN_vkCmdEndDebugUtilsLabelEXT cmd_end_debug_label = nullptr;
	PFN_vkSetDebugUtilsObjectNameEXT set_debug_name = nullptr;
	PFN_vkGetGeneratedCommandsMemoryRequirementsEXT get_generated_commands_memory_requirements = nullptr;
	PFN_vkCmdExecuteGeneratedCommandsEXT cmd_execute_generated_commands = nullptr;
	PFN_vkCreateIndirectCommandsLayoutEXT create_indirect_commands_layout = nullptr;
	PFN_vkDestroyIndirectCommandsLayoutEXT destroy_indirect_commands_layout = nullptr;

	bool supports_descriptor_indexing = false;
	bool supports_mutable_descriptors = false;
	bool supports_dynamic_rendering = false;
	bool supports_timeline_semaphore = false;
	bool supports_maintenance5 = false;
	bool supports_device_generated_commands = false;
	VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT device_generated_commands_properties = {
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT
	};
};

struct vk_shader_stage_data {
	VkShaderModule module = VK_NULL_HANDLE;
	std::string path;
	vk_sampler_kind sampler_kind = vk_sampler_kind::none;
};

struct vk_shader_data {
	vk_shader_stage_data vertex = {};
	vk_shader_stage_data fragment = {};
	vk_shader_stage_data compute = {};
};

struct vk_pipeline_binding_info {
	uint32_t binding = 0u;
	VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
};

struct vk_pipeline_data {
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkDescriptorSetLayout explicit_set_layout = VK_NULL_HANDLE;
	VkDescriptorSetLayout sampler_set_layout = VK_NULL_HANDLE;
	VkDescriptorSet sampler_set = VK_NULL_HANDLE;
	std::vector<vk_pipeline_binding_info> explicit_bindings;
	std::string debug_name;
	bool has_push_constants = false;
	size_t push_constant_count = 0u;
	VkShaderStageFlags push_constant_stage_flags = 0u;
	bool uses_sampler = false;
	vk_sampler_kind sampler_kind = vk_sampler_kind::none;
};

struct vk_compute_pipeline_data {
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkDescriptorSetLayout explicit_set_layout = VK_NULL_HANDLE;
	VkDescriptorSetLayout sampler_set_layout = VK_NULL_HANDLE;
	VkDescriptorSet sampler_set = VK_NULL_HANDLE;
	std::vector<vk_pipeline_binding_info> explicit_bindings;
	bool has_push_constants = false;
	size_t push_constant_count = 0u;
	VkShaderStageFlags push_constant_stage_flags = 0u;
	bool uses_sampler = false;
	vk_sampler_kind sampler_kind = vk_sampler_kind::none;
};

struct vk_command_pool_data {
	VkCommandPool command_pool = VK_NULL_HANDLE;
	std::vector<struct vk_command_buffer_data*> command_buffers;
};

struct vk_command_buffer_data {
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	VkFence submit_fence = VK_NULL_HANDLE;
	vk_command_pool_data* pool = nullptr;
	vk_device_data* device_data = nullptr;
	vk_framebuffer_data* last_bound_framebuffer = nullptr;
	struct vk_depth_buffer_data* last_bound_depth_buffer = nullptr;
	vk_swapchain_data* swapchain = nullptr;
	bool submitted = false;
	bool has_global_memory_barrier = false;
};

struct vk_buffer_data {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	void* mapped_ptr = nullptr;
	size_t size = 0u;
	VkAccessFlags current_access = 0u;
	VkPipelineStageFlags current_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkBufferUsageFlags usage = 0u;
	uint32_t uav_bindless_idx = std::numeric_limits<uint32_t>::max();
	uint32_t srv_bindless_idx = std::numeric_limits<uint32_t>::max();
	bool host_visible = false;
	VkDeviceAddress device_address = 0u;
};

struct vk_texture_data {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView srv_view = VK_NULL_HANDLE;
	std::vector<VkImageView> uav_views;

	VkBuffer upload_buffer = VK_NULL_HANDLE;
	VkDeviceMemory upload_memory = VK_NULL_HANDLE;
	void* upload_mapped = nullptr;
	size_t upload_size = 0u;

	VkFormat format = VK_FORMAT_UNDEFINED;
	mars_format_type mars_format = MARS_FORMAT_UNDEFINED;
	mars_texture_type texture_type = MARS_TEXTURE_TYPE_2D;
	size_t mip_levels = 1u;
	size_t array_size = 1u;
	VkImageLayout current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkAccessFlags current_access = 0u;
	VkPipelineStageFlags current_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	uint32_t srv_bindless_idx = std::numeric_limits<uint32_t>::max();
	uint32_t uav_bindless_base = std::numeric_limits<uint32_t>::max();
	uint32_t uav_descriptor_count = 0u;
};

struct vk_render_pass_data {
	std::vector<mars_format_type> color_formats;
	mars_depth_format depth_format = MARS_DEPTH_FORMAT_UNDEFINED;
	mars_render_pass_load_op load_operation = MARS_RENDER_PASS_LOAD_OP_CLEAR;
	float depth_clear_value = 1.0f;
	std::vector<VkFormat> actual_color_formats;
};

struct vk_depth_buffer_data {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView srv_view = VK_NULL_HANDLE;
	VkImageView dsv_view = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_UNDEFINED;
	mars_depth_format depth_format = MARS_DEPTH_FORMAT_UNDEFINED;
	VkImageLayout current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkAccessFlags current_access = 0u;
	VkPipelineStageFlags current_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	uint32_t srv_bindless_idx = std::numeric_limits<uint32_t>::max();
	bool sampled = true;
};

struct vk_framebuffer_data {
	std::vector<VkImage> color_images;
	std::vector<VkImageView> color_views;
	bool is_swapchain = false;
	uint32_t swapchain_image_index = 0u;
	vk_swapchain_data* swapchain_owner = nullptr;
	std::vector<vk_texture_data*> color_textures;
};

struct vk_descriptor_data {
	VkDescriptorPool pool = VK_NULL_HANDLE;
	size_t frames_in_flight = 1u;
};

struct vk_descriptor_set_data {
	vk_device_data* device_data = nullptr;
	std::vector<VkDescriptorSet> sets;
	std::vector<vk_pipeline_binding_info> bindings;
};

struct vk_timeline_fence_data {
	VkDevice device = VK_NULL_HANDLE;
	VkSemaphore semaphore = VK_NULL_HANDLE;
	std::atomic<uint64_t> next_value{1u};
};

struct vk_upload_ring_data {
	vk_device_data* device_data = nullptr;
	vk_queue_data* queue = nullptr;
	VkBuffer upload_buffer = VK_NULL_HANDLE;
	VkDeviceMemory upload_memory = VK_NULL_HANDLE;
	uint8_t* mapped_ptr = nullptr;
	size_t capacity = 0u;
	size_t write_head = 0u;
	VkCommandPool command_pool = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	VkFence fence = VK_NULL_HANDLE;
	bool recording = false;
	uint64_t next_fence_value = 1u;
};

struct vk_readback_buffer_data {
	vk_device_data* device_data = nullptr;
	vk_queue_data* queue = nullptr;
	VkBuffer readback_buffer = VK_NULL_HANDLE;
	VkDeviceMemory readback_memory = VK_NULL_HANDLE;
	uint8_t* mapped_ptr = nullptr;
	size_t slot_size = 0u;
	uint32_t num_slots = 0u;
	uint32_t current_slot = 0u;
	VkCommandPool command_pool = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	VkFence fence = VK_NULL_HANDLE;
	bool pending = false;
	uint64_t next_fence_value = 1u;

	struct pending_batch {
		uint32_t slot_index = 0u;
		size_t data_size = 0u;
		uint64_t fence_value = 0u;
	};
	std::deque<pending_batch> in_flight;
};

struct vk_indirect_executor_data {
	vk_device_data* device_data = nullptr;
	mars_command_signature_type type = MARS_COMMAND_SIGNATURE_DISPATCH;
	VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
	VkIndirectCommandsLayoutEXT indirect_layout = VK_NULL_HANDLE;
	VkShaderStageFlags shader_stages = 0u;
	uint32_t embedded_constant_bytes = 0u;
	uint32_t command_stride = 0u;
	uint32_t command_offset = 0u;
	bool has_embedded_constants = false;
	struct preprocess_buffer_entry {
		struct vk_command_buffer_data* command_buffer = nullptr;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDeviceAddress device_address = 0u;
		VkDeviceSize size = 0u;
		uint32_t max_sequence_count = 0u;
	};
	std::vector<preprocess_buffer_entry> preprocess_buffers;
};

struct vk_append_buffer_data {
	mars::buffer data_buf = {};
	mars::buffer counter_buf = {};
	mars::buffer zero_buf = {};
};

struct vk_swapchain_data {
	vk_device_data* device_data = nullptr;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
	VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkExtent2D extent = {};
	std::vector<VkImage> images;
	std::vector<VkImageView> views;
	std::vector<VkImageLayout> image_layouts;
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	VkSemaphore active_image_available_semaphore = VK_NULL_HANDLE;
	VkSemaphore active_render_finished_semaphore = VK_NULL_HANDLE;
	size_t sync_index = 0u;
	uint32_t acquired_image_index = 0u;
	bool image_acquired = false;
};

inline VkFormat vk_format_from_mars(mars_format_type format) {
	switch (format) {
	case MARS_FORMAT_R8_UNORM:
		return VK_FORMAT_R8_UNORM;
	case MARS_FORMAT_RG8_UNORM:
		return VK_FORMAT_R8G8_UNORM;
	case MARS_FORMAT_RGBA8_UNORM:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case MARS_FORMAT_RGBA8_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case MARS_FORMAT_BGRA8_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case MARS_FORMAT_R32_SFLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case MARS_FORMAT_RG32_SFLOAT:
		return VK_FORMAT_R32G32_SFLOAT;
	case MARS_FORMAT_RGB32_SFLOAT:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case MARS_FORMAT_RGBA32_SFLOAT:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case MARS_FORMAT_R32_UINT:
		return VK_FORMAT_R32_UINT;
	case MARS_FORMAT_RG32_UINT:
		return VK_FORMAT_R32G32_UINT;
	case MARS_FORMAT_RGB32_UINT:
		return VK_FORMAT_R32G32B32_UINT;
	case MARS_FORMAT_RGBA32_UINT:
		return VK_FORMAT_R32G32B32A32_UINT;
	case MARS_FORMAT_RGBA16_SFLOAT:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case MARS_FORMAT_R32_TYPELESS:
		return VK_FORMAT_UNDEFINED;
	default:
		return VK_FORMAT_UNDEFINED;
	}
}

inline VkFormat vk_depth_format_from_mars(mars_depth_format format) {
	switch (format) {
	case MARS_DEPTH_FORMAT_D32_SFLOAT:
		return VK_FORMAT_D32_SFLOAT;
	case MARS_DEPTH_FORMAT_UNDEFINED:
	default:
		return VK_FORMAT_UNDEFINED;
	}
}

inline size_t vk_format_pixel_size(mars_format_type format) {
	switch (format) {
	case MARS_FORMAT_R8_UNORM:
		return 1u;
	case MARS_FORMAT_RG8_UNORM:
		return 2u;
	case MARS_FORMAT_RGBA8_UNORM:
	case MARS_FORMAT_RGBA8_SRGB:
	case MARS_FORMAT_BGRA8_SRGB:
	case MARS_FORMAT_R32_SFLOAT:
	case MARS_FORMAT_R32_UINT:
		return 4u;
	case MARS_FORMAT_RG32_SFLOAT:
	case MARS_FORMAT_RG32_UINT:
	case MARS_FORMAT_RGBA16_SFLOAT:
		return 8u;
	case MARS_FORMAT_RGB32_SFLOAT:
	case MARS_FORMAT_RGB32_UINT:
		return 12u;
	case MARS_FORMAT_RGBA32_SFLOAT:
	case MARS_FORMAT_RGBA32_UINT:
		return 16u;
	default:
		return 4u;
	}
}

inline VkShaderStageFlags vk_shader_stage_flags(mars_pipeline_stage stage) {
	VkShaderStageFlags flags = 0u;
	if (mars::enum_has_flag(stage, MARS_PIPELINE_STAGE_VERTEX))
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	if (mars::enum_has_flag(stage, MARS_PIPELINE_STAGE_FRAGMENT))
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if (mars::enum_has_flag(stage, MARS_PIPELINE_STAGE_COMPUTE))
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	return flags != 0u ? flags : VK_SHADER_STAGE_ALL;
}

inline VkDescriptorType vk_descriptor_type_from_pipeline_descriptor(mars_pipeline_descriptor_type descriptor_type) {
	switch (descriptor_type) {
	case MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case MARS_PIPELINE_DESCRIPTOR_TYPE_IMAGE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case MARS_PIPELINE_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	default:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
}

inline VkCompareOp vk_compare_op_from_mars(mars_compare_op op) {
	switch (op) {
	case MARS_COMPARE_OP_NEVER:
		return VK_COMPARE_OP_NEVER;
	case MARS_COMPARE_OP_LESS:
		return VK_COMPARE_OP_LESS;
	case MARS_COMPARE_OP_EQUAL:
		return VK_COMPARE_OP_EQUAL;
	case MARS_COMPARE_OP_LESS_EQUAL:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case MARS_COMPARE_OP_GREATER:
		return VK_COMPARE_OP_GREATER;
	case MARS_COMPARE_OP_NOT_EQUAL:
		return VK_COMPARE_OP_NOT_EQUAL;
	case MARS_COMPARE_OP_GREATER_EQUAL:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case MARS_COMPARE_OP_ALWAYS:
	default:
		return VK_COMPARE_OP_ALWAYS;
	}
}

inline uint32_t vk_find_memory_type(const vk_device_data* device_data, uint32_t type_bits, VkMemoryPropertyFlags properties) {
	for (uint32_t index = 0u; index < device_data->memory_properties.memoryTypeCount; ++index) {
		const bool supported = (type_bits & (1u << index)) != 0u;
		const bool matches = (device_data->memory_properties.memoryTypes[index].propertyFlags & properties) == properties;
		if (supported && matches)
			return index;
	}
	mars::logger::error(vk_log_channel(), "Failed to find Vulkan memory type (bits={}, properties={:#x})", type_bits, static_cast<unsigned>(properties));
	return 0u;
}

inline VkDescriptorType vk_bindless_descriptor_type_for_buffer(const buffer_create_params&) {
	return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
}

inline void vk_merge_bindless_ranges(std::vector<std::pair<uint32_t, uint32_t>>& ranges) {
	if (ranges.empty())
		return;
	std::sort(ranges.begin(), ranges.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
	size_t write_index = 0u;
	for (size_t read_index = 1u; read_index < ranges.size(); ++read_index) {
		auto& current = ranges[write_index];
		const auto& next = ranges[read_index];
		const uint32_t current_end = current.first + current.second;
		if (next.first <= current_end) {
			const uint32_t next_end = next.first + next.second;
			current.second = (std::max)(current_end, next_end) - current.first;
			continue;
		}
		ranges[++write_index] = next;
	}
	ranges.resize(write_index + 1u);
}

inline uint32_t vk_allocate_bindless_range(
	std::vector<std::pair<uint32_t, uint32_t>>& free_ranges,
	uint32_t& next_slot,
	uint32_t limit_exclusive,
	uint32_t count,
	const char* label
) {
	assert(count > 0u);

	for (size_t index = 0u; index < free_ranges.size(); ++index) {
		auto& range = free_ranges[index];
		if (range.second < count)
			continue;
		const uint32_t start = range.first;
		range.first += count;
		range.second -= count;
		if (range.second == 0u)
			free_ranges.erase(free_ranges.begin() + static_cast<std::ptrdiff_t>(index));
		return start;
	}

	if (next_slot > limit_exclusive || count > (limit_exclusive - next_slot)) {
		mars::logger::error(vk_log_channel(), "Bindless {} descriptor heap exhausted", label);
		assert(false && "Bindless descriptor heap exhausted");
		return std::numeric_limits<uint32_t>::max();
	}

	const uint32_t start = next_slot;
	next_slot += count;
	return start;
}

inline void vk_release_bindless_range(std::vector<std::pair<uint32_t, uint32_t>>& free_ranges, uint32_t start, uint32_t count) {
	if (start == std::numeric_limits<uint32_t>::max() || count == 0u)
		return;
	free_ranges.push_back({start, count});
	vk_merge_bindless_ranges(free_ranges);
}

inline uint32_t vk_allocate_bindless_srv_slot(vk_device_data* device_data) {
	return vk_allocate_bindless_range(
		device_data->free_bindless_srv_ranges,
		device_data->next_bindless_srv_idx,
		vk_device_data::BINDLESS_SRV_BASE + vk_device_data::BINDLESS_SRV_COUNT,
		1u,
		"srv"
	);
}

inline uint32_t vk_allocate_bindless_uav_range(vk_device_data* device_data, uint32_t count) {
	return vk_allocate_bindless_range(
		device_data->free_bindless_uav_ranges,
		device_data->next_bindless_uav_idx,
		vk_device_data::BINDLESS_UAV_BASE + vk_device_data::BINDLESS_UAV_COUNT,
		count,
		"uav"
	);
}

inline void vk_release_bindless_srv_slot(vk_device_data* device_data, uint32_t slot) {
	vk_release_bindless_range(device_data->free_bindless_srv_ranges, slot, 1u);
}

inline void vk_release_bindless_uav_range(vk_device_data* device_data, uint32_t start, uint32_t count) {
	vk_release_bindless_range(device_data->free_bindless_uav_ranges, start, count);
}

struct vk_buffer_barrier_state {
	VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkAccessFlags access = 0u;
};

inline vk_buffer_barrier_state vk_buffer_barrier_state_from_mars(mars_buffer_state state) {
	switch (state) {
	case MARS_BUFFER_STATE_SHADER_READ:
		return {VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT};
	case MARS_BUFFER_STATE_UNORDERED_ACCESS:
		return {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT};
	case MARS_BUFFER_STATE_COPY_SOURCE:
		return {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT};
	case MARS_BUFFER_STATE_INDIRECT_ARGUMENT:
		return {
			VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
				VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_EXT |
				VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
				VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_EXT |
				VK_ACCESS_SHADER_READ_BIT
		};
	case MARS_BUFFER_STATE_COMMON:
	default:
		return {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0u};
	}
}

struct vk_texture_barrier_state {
	VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkAccessFlags access = 0u;
	VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL;
};

inline vk_texture_barrier_state vk_texture_barrier_state_from_mars(mars_texture_state state) {
	switch (state) {
	case MARS_TEXTURE_STATE_COPY_DST:
		return {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};
	case MARS_TEXTURE_STATE_SHADER_READ:
		return {VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};
	case MARS_TEXTURE_STATE_UNORDERED_ACCESS:
		return {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL};
	case MARS_TEXTURE_STATE_COMMON:
	default:
		return {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0u, VK_IMAGE_LAYOUT_GENERAL};
	}
}

inline VkPipelineStageFlags vk_attachment_stage_flags() {
	return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
}

inline VkAccessFlags vk_attachment_access_flags() {
	return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
}

inline void vk_destroy_buffer(vk_device_data* device_data, VkBuffer buffer, VkDeviceMemory memory) {
	if (buffer != VK_NULL_HANDLE)
		vkDestroyBuffer(device_data->device, buffer, nullptr);
	if (memory != VK_NULL_HANDLE)
		vkFreeMemory(device_data->device, memory, nullptr);
}

inline void vk_destroy_image(vk_device_data* device_data, VkImage image, VkImageView view, VkDeviceMemory memory) {
	if (view != VK_NULL_HANDLE)
		vkDestroyImageView(device_data->device, view, nullptr);
	if (image != VK_NULL_HANDLE)
		vkDestroyImage(device_data->device, image, nullptr);
	if (memory != VK_NULL_HANDLE)
		vkFreeMemory(device_data->device, memory, nullptr);
}

inline std::optional<vk_pipeline_binding_info> vk_find_binding_info(const std::vector<vk_pipeline_binding_info>& bindings, uint32_t binding) {
	for (const auto& entry : bindings)
		if (entry.binding == binding)
			return entry;
	return std::nullopt;
}

inline vk_queue_data* vk_get_queue(vk_device_data* device_data, mars_command_queue_type queue_type) {
	switch (queue_type) {
	case MARS_COMMAND_QUEUE_COMPUTE:
		return &device_data->compute_queue;
	case MARS_COMMAND_QUEUE_COPY:
		return &device_data->copy_queue;
	case MARS_COMMAND_QUEUE_DIRECT:
	default:
		return &device_data->direct_queue;
	}
}

inline VkSampler vk_get_sampler(vk_device_data* device_data, vk_sampler_kind sampler_kind) {
	switch (sampler_kind) {
	case vk_sampler_kind::point_clamp:
		return device_data->point_clamp_sampler;
	case vk_sampler_kind::linear_clamp:
		return device_data->linear_clamp_sampler;
	case vk_sampler_kind::none:
	default:
		return VK_NULL_HANDLE;
	}
}

} // namespace mars::graphics::vk

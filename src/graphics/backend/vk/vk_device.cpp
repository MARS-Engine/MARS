#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_backend.hpp>
#include <mars/graphics/backend/vk/vk_device.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>

#include <SDL3/SDL_vulkan.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <set>
#include <string>
#include <vector>

namespace mars::graphics::vk {
namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void*) {
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		mars::logger::assert_(vk_log_channel(), "{}", callback_data->pMessage);
	else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		mars::logger::warning(vk_log_channel(), "{}", callback_data->pMessage);
	else
		mars::logger::log(vk_log_channel(), "{}", callback_data->pMessage);
	return VK_FALSE;
}

struct queue_family_selection {
	uint32_t graphics = std::numeric_limits<uint32_t>::max();
	uint32_t compute = std::numeric_limits<uint32_t>::max();
	uint32_t transfer = std::numeric_limits<uint32_t>::max();
};

queue_family_selection select_queue_families(VkPhysicalDevice physical_device) {
	uint32_t queue_family_count = 0u;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, families.data());

	queue_family_selection selected = {};
	for (uint32_t index = 0u; index < queue_family_count; ++index) {
		const auto& family = families[index];
		if (selected.graphics == std::numeric_limits<uint32_t>::max() && (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u)
			selected.graphics = index;
		if (selected.compute == std::numeric_limits<uint32_t>::max() &&
			(family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u &&
			(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0u)
			selected.compute = index;
		if (selected.transfer == std::numeric_limits<uint32_t>::max() &&
			(family.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u &&
			(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0u &&
			(family.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0u)
			selected.transfer = index;
	}

	if (selected.compute == std::numeric_limits<uint32_t>::max())
		selected.compute = selected.graphics;
	if (selected.transfer == std::numeric_limits<uint32_t>::max())
		selected.transfer = selected.compute != std::numeric_limits<uint32_t>::max() ? selected.compute : selected.graphics;
	return selected;
}

bool supports_required_extensions(VkPhysicalDevice physical_device, const std::vector<const char*>& required_extensions) {
	uint32_t extension_count = 0u;
	vk_expect<vkEnumerateDeviceExtensionProperties>(physical_device, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> extensions(extension_count);
	vk_expect<vkEnumerateDeviceExtensionProperties>(physical_device, nullptr, &extension_count, extensions.data());

	for (const char* required : required_extensions) {
		const bool found = std::any_of(extensions.begin(), extensions.end(), [required](const VkExtensionProperties& extension) {
			return std::strcmp(extension.extensionName, required) == 0;
		});
		if (!found)
			return false;
	}
	return true;
}

bool validation_layer_available(const char* layer_name) {
	uint32_t layer_count = 0u;
	vk_expect<vkEnumerateInstanceLayerProperties>(&layer_count, nullptr);
	std::vector<VkLayerProperties> layers(layer_count);
	vk_expect<vkEnumerateInstanceLayerProperties>(&layer_count, layers.data());
	return std::any_of(layers.begin(), layers.end(), [layer_name](const VkLayerProperties& layer) {
		return std::strcmp(layer.layerName, layer_name) == 0;
	});
}

bool query_required_features(vk_device_data* data, VkPhysicalDevice physical_device, bool require_generated_commands, std::string& failure_reason) {
	VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamic_rendering_unused_attachments = {};
	dynamic_rendering_unused_attachments.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT;

	VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutable_descriptor = {};
	mutable_descriptor.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT;
	mutable_descriptor.pNext = &dynamic_rendering_unused_attachments;

	VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamic_rendering_local_read = {};
	dynamic_rendering_local_read.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR;
	dynamic_rendering_local_read.pNext = &mutable_descriptor;

	VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT generated_commands = {};
	generated_commands.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT;

	VkPhysicalDeviceMaintenance5Features maintenance5 = {};
	maintenance5.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES;

	if (require_generated_commands) {
	    generated_commands.pNext = &dynamic_rendering_local_read;
	    maintenance5.pNext = &generated_commands;
	} 
	else {
	    maintenance5.pNext = &dynamic_rendering_local_read;
	    data->device_generated_commands_properties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT};
	}

	VkPhysicalDeviceVulkan12Features features_12 = {};
	features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features_12.pNext = &maintenance5;

	VkPhysicalDeviceVulkan11Features features_11 = {};
	features_11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	features_11.pNext = &features_12;

	VkPhysicalDeviceVulkan13Features features_13 = {};
	features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	features_13.pNext = &features_11;

	VkPhysicalDeviceFeatures2 features_2 = {};
	features_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features_2.pNext = &features_13;
	vkGetPhysicalDeviceFeatures2(physical_device, &features_2);

	VkPhysicalDeviceProperties2 properties_2 = {};
	properties_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties_2.pNext = require_generated_commands ? &data->device_generated_commands_properties : nullptr;
	vkGetPhysicalDeviceProperties2(physical_device, &properties_2);

	if (!features_12.timelineSemaphore) {
		failure_reason = "timelineSemaphore is required";
		return false;
	}
	if (!features_12.drawIndirectCount) {
		failure_reason = "drawIndirectCount is required";
		return false;
	}
	if (!features_11.shaderDrawParameters) {
		failure_reason = "shaderDrawParameters is required";
		return false;
	}
	if (!features_12.shaderBufferInt64Atomics) {
		failure_reason = "shaderBufferInt64Atomics is required";
		return false;
	}
	if (!features_12.descriptorIndexing) {
		failure_reason = "descriptorIndexing is required";
		return false;
	}
	if (!features_12.bufferDeviceAddress) {
		failure_reason = "bufferDeviceAddress is required";
		return false;
	}
	if (!features_12.runtimeDescriptorArray ||
		!features_12.descriptorBindingPartiallyBound ||
		!features_12.shaderSampledImageArrayNonUniformIndexing ||
		!features_12.shaderStorageImageArrayNonUniformIndexing ||
		!features_12.shaderStorageBufferArrayNonUniformIndexing ||
		!features_12.shaderUniformBufferArrayNonUniformIndexing) {
		failure_reason = "descriptor indexing features are incomplete";
		return false;
	}
	if (!features_12.scalarBlockLayout) {
		failure_reason = "scalarBlockLayout is required";
		return false;
	}
	if (!features_13.dynamicRendering) {
		failure_reason = "dynamicRendering is required";
		return false;
	}
	if (!dynamic_rendering_unused_attachments.dynamicRenderingUnusedAttachments) {
		failure_reason = "dynamicRenderingUnusedAttachments is required";
		return false;
	}
	if (!features_13.synchronization2) {
		failure_reason = "synchronization2 is required";
		return false;
	}
	if (!features_13.shaderDemoteToHelperInvocation) {
		failure_reason = "shaderDemoteToHelperInvocation is required";
		return false;
	}
	if (!maintenance5.maintenance5) {
		failure_reason = "maintenance5 is required";
		return false;
	}
	if (require_generated_commands && !generated_commands.deviceGeneratedCommands) {
		failure_reason = "deviceGeneratedCommands is required";
		return false;
	}
	if (!mutable_descriptor.mutableDescriptorType) {
		failure_reason = "mutableDescriptorType is required";
		return false;
	}
	if (!features_2.features.geometryShader) {
		failure_reason = "geometryShader is required";
		return false;
	}
	if (!features_2.features.multiDrawIndirect) {
		failure_reason = "multiDrawIndirect is required";
		return false;
	}
	if (!features_2.features.drawIndirectFirstInstance) {
		failure_reason = "drawIndirectFirstInstance is required";
		return false;
	}
	if (!features_2.features.fragmentStoresAndAtomics) {
		failure_reason = "fragmentStoresAndAtomics is required";
		return false;
	}

	if (require_generated_commands && data->device_generated_commands_properties.maxIndirectSequenceCount == 0u) {
		failure_reason = "device generated commands reports maxIndirectSequenceCount == 0";
		return false;
	}

	if (!dynamic_rendering_local_read.dynamicRenderingLocalRead) {
    	failure_reason = "dynamicRenderingLocalRead is required";
    	return false;
	}

	return true;
}

VkPhysicalDevice select_physical_device(VkInstance instance, const std::vector<const char*>& required_extensions) {
	uint32_t physical_device_count = 0u;
	vk_expect<vkEnumeratePhysicalDevices>(instance, &physical_device_count, nullptr);
	if (physical_device_count == 0u)
		return VK_NULL_HANDLE;
	std::vector<VkPhysicalDevice> devices(physical_device_count);
	vk_expect<vkEnumeratePhysicalDevices>(instance, &physical_device_count, devices.data());

	VkPhysicalDevice best_device = VK_NULL_HANDLE;
	VkPhysicalDeviceType best_type = VK_PHYSICAL_DEVICE_TYPE_OTHER;
	for (VkPhysicalDevice candidate : devices) {
		if (!supports_required_extensions(candidate, required_extensions))
			continue;

		const auto queues = select_queue_families(candidate);
		if (queues.graphics == std::numeric_limits<uint32_t>::max())
			continue;

		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(candidate, &properties);
		if (best_device == VK_NULL_HANDLE || properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || best_type != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			best_device = candidate;
			best_type = properties.deviceType;
		}
	}
	return best_device;
}

VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance instance) {
	auto create_fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
	);
	if (!create_fn)
		return VK_NULL_HANDLE;

	VkDebugUtilsMessengerCreateInfoEXT create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = &debug_messenger_callback;

	VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
	vk_expect(create_fn, instance, &create_info, nullptr, &messenger);
	return messenger;
}

VkDescriptorSetLayout create_empty_set_layout(VkDevice device) {
	VkDescriptorSetLayoutCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	vk_expect<vkCreateDescriptorSetLayout>(device, &create_info, nullptr, &layout);
	return layout;
}

VkDescriptorPool create_empty_descriptor_pool(VkDevice device) {
	VkDescriptorPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.maxSets = 1u;
	VkDescriptorPool pool = VK_NULL_HANDLE;
	vk_expect<vkCreateDescriptorPool>(device, &create_info, nullptr, &pool);
	return pool;
}

VkDescriptorSet allocate_descriptor_set(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout, const uint32_t* variable_descriptor_count = nullptr) {
	VkDescriptorSetVariableDescriptorCountAllocateInfo variable_info = {};
	if (variable_descriptor_count) {
		variable_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		variable_info.descriptorSetCount = 1u;
		variable_info.pDescriptorCounts = variable_descriptor_count;
	}

	VkDescriptorSetAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.pNext = variable_descriptor_count ? &variable_info : nullptr;
	allocate_info.descriptorPool = pool;
	allocate_info.descriptorSetCount = 1u;
	allocate_info.pSetLayouts = &layout;

	VkDescriptorSet set = VK_NULL_HANDLE;
	vk_expect<vkAllocateDescriptorSets>(device, &allocate_info, &set);
	return set;
}

VkSampler create_sampler(VkDevice device, VkFilter filter) {
	VkSamplerCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.magFilter = filter;
	create_info.minFilter = filter;
	create_info.mipmapMode = filter == VK_FILTER_NEAREST ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
	create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	create_info.minLod = 0.0f;
	create_info.maxLod = VK_LOD_CLAMP_NONE;
	create_info.maxAnisotropy = 1.0f;
	VkSampler sampler = VK_NULL_HANDLE;
	vk_expect<vkCreateSampler>(device, &create_info, nullptr, &sampler);
	return sampler;
}

void create_descriptor_infrastructure(vk_device_data* data) {
	const std::array<VkDescriptorType, 3> mutable_types = {
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	};
	VkMutableDescriptorTypeListEXT mutable_type_list = {};
	mutable_type_list.descriptorTypeCount = static_cast<uint32_t>(mutable_types.size());
	mutable_type_list.pDescriptorTypes = mutable_types.data();

	VkMutableDescriptorTypeCreateInfoEXT mutable_info = {};
	mutable_info.sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT;
	mutable_info.mutableDescriptorTypeListCount = 1u;
	mutable_info.pMutableDescriptorTypeLists = &mutable_type_list;

	VkDescriptorSetLayoutBinding bindless_binding = {};
	bindless_binding.binding = 0u;
	bindless_binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
	bindless_binding.descriptorCount = vk_device_data::BINDLESS_TOTAL;
	bindless_binding.stageFlags = VK_SHADER_STAGE_ALL;

	const VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
	VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info = {};
	binding_flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	binding_flags_info.bindingCount = 1u;
	binding_flags_info.pBindingFlags = &binding_flags;

	VkDescriptorSetLayoutCreateInfo set_layout_info = {};
	set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout_info.pNext = &binding_flags_info;
	binding_flags_info.pNext = &mutable_info;
	set_layout_info.bindingCount = 1u;
	set_layout_info.pBindings = &bindless_binding;
	vk_expect<vkCreateDescriptorSetLayout>(data->device, &set_layout_info, nullptr, &data->bindless_set_layout);

	std::array<VkDescriptorPoolSize, 1> pool_sizes = {{
		{VK_DESCRIPTOR_TYPE_MUTABLE_EXT, vk_device_data::BINDLESS_TOTAL},
	}};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.pNext = &mutable_info;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1u;
	pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_info.pPoolSizes = pool_sizes.data();
	vk_expect<vkCreateDescriptorPool>(data->device, &pool_info, nullptr, &data->bindless_pool);

	data->bindless_set = allocate_descriptor_set(data->device, data->bindless_pool, data->bindless_set_layout);

	data->empty_set_layout = create_empty_set_layout(data->device);
	data->empty_descriptor_pool = create_empty_descriptor_pool(data->device);
	data->empty_set = allocate_descriptor_set(data->device, data->empty_descriptor_pool, data->empty_set_layout);

	VkDescriptorPoolSize sampler_pool_size = {VK_DESCRIPTOR_TYPE_SAMPLER, 512u};
	VkDescriptorPoolCreateInfo sampler_pool_info = {};
	sampler_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	sampler_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	sampler_pool_info.maxSets = 512u;
	sampler_pool_info.poolSizeCount = 1u;
	sampler_pool_info.pPoolSizes = &sampler_pool_size;
	vk_expect<vkCreateDescriptorPool>(data->device, &sampler_pool_info, nullptr, &data->sampler_descriptor_pool);

	data->linear_clamp_sampler = create_sampler(data->device, VK_FILTER_LINEAR);
	data->point_clamp_sampler = create_sampler(data->device, VK_FILTER_NEAREST);
}

} // namespace

device vk_device_impl::vk_device_create(graphics_engine& _engine) {
	auto* data = new vk_device_data();

	SDL_Vulkan_LoadLibrary(nullptr);

	std::vector<const char*> instance_extensions;
	{
		Uint32 sdl_extension_count = 0u;
		const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count);
		for (Uint32 index = 0u; index < sdl_extension_count; ++index)
			instance_extensions.push_back(sdl_extensions[index]);
	}

	instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "mars";
	app_info.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
	app_info.pEngineName = "mars";
	app_info.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
	app_info.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instance_info = {};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
	instance_info.ppEnabledExtensionNames = instance_extensions.data();

	const char* validation_layer = "VK_LAYER_KHRONOS_validation";
	if (validation_layer_available(validation_layer)) {
		instance_info.enabledLayerCount = 1u;
		instance_info.ppEnabledLayerNames = &validation_layer;
	}

	vk_expect<vkCreateInstance>(&instance_info, nullptr, &data->instance);
	data->debug_messenger = create_debug_messenger(data->instance);

	data->set_debug_name = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(data->instance, "vkSetDebugUtilsObjectNameEXT"));

	const std::vector<const char*> base_device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME,
		VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME,
		VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
	};
	std::vector<const char*> device_extensions = base_device_extensions;
	device_extensions.push_back(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);

	data->physical_device = select_physical_device(data->instance, device_extensions);
	const bool enable_generated_commands = data->physical_device != VK_NULL_HANDLE;
	if (!enable_generated_commands)
		data->physical_device = select_physical_device(data->instance, base_device_extensions);
	mars::logger::assert_(data->physical_device != VK_NULL_HANDLE, vk_log_channel(), "Failed to select a Vulkan physical device");

	std::string feature_failure_reason;
	mars::logger::assert_(
		query_required_features(data, data->physical_device, enable_generated_commands, feature_failure_reason),
		vk_log_channel(),
		"Selected Vulkan device does not satisfy backend requirements: {}",
		feature_failure_reason
	);

	vkGetPhysicalDeviceProperties(data->physical_device, &data->physical_device_properties);
	vkGetPhysicalDeviceMemoryProperties(data->physical_device, &data->memory_properties);

	const auto queue_families = select_queue_families(data->physical_device);
	std::set<uint32_t> unique_queue_families = {
		queue_families.graphics,
		queue_families.compute,
		queue_families.transfer,
	};
	const float queue_priority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queue_infos;
	for (uint32_t family_index : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_info = {};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info.queueFamilyIndex = family_index;
		queue_info.queueCount = 1u;
		queue_info.pQueuePriorities = &queue_priority;
		queue_infos.push_back(queue_info);
	}

	VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamic_rendering_unused_attachments = {};
	dynamic_rendering_unused_attachments.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT;
	dynamic_rendering_unused_attachments.dynamicRenderingUnusedAttachments = VK_TRUE;

	VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutable_descriptor_features = {};
	mutable_descriptor_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT;
	mutable_descriptor_features.mutableDescriptorType = VK_TRUE;
	mutable_descriptor_features.pNext = &dynamic_rendering_unused_attachments;

	VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamic_rendering_local_read_features = {};
	dynamic_rendering_local_read_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR;
	dynamic_rendering_local_read_features.dynamicRenderingLocalRead = VK_TRUE;
	dynamic_rendering_local_read_features.pNext = &mutable_descriptor_features;

	VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT generated_commands_features = {};
	generated_commands_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT;

	VkPhysicalDeviceMaintenance5Features maintenance5_features = {};
	maintenance5_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES;
	maintenance5_features.maintenance5 = VK_TRUE;

	if (enable_generated_commands) {
		generated_commands_features.deviceGeneratedCommands = VK_TRUE;
		generated_commands_features.dynamicGeneratedPipelineLayout = VK_FALSE;
		generated_commands_features.pNext = &dynamic_rendering_local_read_features;
		maintenance5_features.pNext = &generated_commands_features;
	} else
		maintenance5_features.pNext = &dynamic_rendering_local_read_features;

	VkPhysicalDeviceVulkan12Features features_12 = {};
	features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features_12.timelineSemaphore = VK_TRUE;
	features_12.drawIndirectCount = VK_TRUE;
	features_12.shaderBufferInt64Atomics = VK_TRUE;
	features_12.descriptorIndexing = VK_TRUE;
	features_12.bufferDeviceAddress = VK_TRUE;
	features_12.runtimeDescriptorArray = VK_TRUE;
	features_12.descriptorBindingPartiallyBound = VK_TRUE;
	features_12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	features_12.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
	features_12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
	features_12.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
	features_12.scalarBlockLayout = VK_TRUE;
	features_12.pNext = &maintenance5_features;

	VkPhysicalDeviceVulkan11Features features_11 = {};
	features_11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	features_11.shaderDrawParameters = VK_TRUE;
	features_11.pNext = &features_12;

	VkPhysicalDeviceVulkan13Features features_13 = {};
	features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	features_13.dynamicRendering = VK_TRUE;
	features_13.synchronization2 = VK_TRUE;
	features_13.shaderDemoteToHelperInvocation = VK_TRUE;
	features_13.pNext = &features_11;

	VkPhysicalDeviceFeatures device_features = {};
	device_features.shaderInt64 = VK_TRUE;
	device_features.geometryShader = VK_TRUE;
	device_features.multiDrawIndirect = VK_TRUE;
	device_features.drawIndirectFirstInstance = VK_TRUE;
	device_features.fragmentStoresAndAtomics = VK_TRUE;
	device_features.fillModeNonSolid = VK_FALSE;

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = &features_13;
	device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
	device_info.pQueueCreateInfos = queue_infos.data();
	device_info.enabledExtensionCount = static_cast<uint32_t>((enable_generated_commands ? device_extensions : base_device_extensions).size());
	device_info.ppEnabledExtensionNames = (enable_generated_commands ? device_extensions : base_device_extensions).data();
	device_info.pEnabledFeatures = &device_features;

	if (validation_layer_available(validation_layer)) {
		device_info.enabledLayerCount = 1u;
		device_info.ppEnabledLayerNames = &validation_layer;
	}

	vk_expect<vkCreateDevice>(data->physical_device, &device_info, nullptr, &data->device);

	vkGetDeviceQueue(data->device, queue_families.graphics, 0u, &data->direct_queue.queue);
	data->direct_queue.family_index = queue_families.graphics;
	vkGetDeviceQueue(data->device, queue_families.compute, 0u, &data->compute_queue.queue);
	data->compute_queue.family_index = queue_families.compute;
	vkGetDeviceQueue(data->device, queue_families.transfer, 0u, &data->copy_queue.queue);
	data->copy_queue.family_index = queue_families.transfer;

	data->supports_descriptor_indexing = true;
	data->supports_mutable_descriptors = true;
	data->supports_dynamic_rendering = true;
	data->supports_timeline_semaphore = true;
	data->supports_maintenance5 = true;
	data->supports_device_generated_commands = enable_generated_commands;

	data->cmd_begin_debug_label = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(data->device, "vkCmdBeginDebugUtilsLabelEXT"));
	data->cmd_end_debug_label = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(data->device, "vkCmdEndDebugUtilsLabelEXT"));
	if (enable_generated_commands) {
		data->get_generated_commands_memory_requirements = reinterpret_cast<PFN_vkGetGeneratedCommandsMemoryRequirementsEXT>(
			vkGetDeviceProcAddr(data->device, "vkGetGeneratedCommandsMemoryRequirementsEXT")
		);
		data->cmd_execute_generated_commands = reinterpret_cast<PFN_vkCmdExecuteGeneratedCommandsEXT>(
			vkGetDeviceProcAddr(data->device, "vkCmdExecuteGeneratedCommandsEXT")
		);
		data->create_indirect_commands_layout = reinterpret_cast<PFN_vkCreateIndirectCommandsLayoutEXT>(
			vkGetDeviceProcAddr(data->device, "vkCreateIndirectCommandsLayoutEXT")
		);
		data->destroy_indirect_commands_layout = reinterpret_cast<PFN_vkDestroyIndirectCommandsLayoutEXT>(
			vkGetDeviceProcAddr(data->device, "vkDestroyIndirectCommandsLayoutEXT")
		);

		mars::logger::assert_(data->get_generated_commands_memory_requirements != nullptr, vk_log_channel(), "Failed to load vkGetGeneratedCommandsMemoryRequirementsEXT");
		mars::logger::assert_(data->cmd_execute_generated_commands != nullptr, vk_log_channel(), "Failed to load vkCmdExecuteGeneratedCommandsEXT");
		mars::logger::assert_(data->create_indirect_commands_layout != nullptr, vk_log_channel(), "Failed to load vkCreateIndirectCommandsLayoutEXT");
		mars::logger::assert_(data->destroy_indirect_commands_layout != nullptr, vk_log_channel(), "Failed to load vkDestroyIndirectCommandsLayoutEXT");
	}

	create_descriptor_infrastructure(data);

	device result;
	result.engine = _engine.allocator;
	result.data.store(data);
	return result;
}

void vk_device_impl::vk_device_submit(const device& _device, const command_buffer& _command_buffer) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1u;
	submit_info.pCommandBuffers = &command_buffer_data->command_buffer;

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	if (command_buffer_data->swapchain && command_buffer_data->swapchain->image_acquired) {
		submit_info.waitSemaphoreCount = 1u;
		submit_info.pWaitSemaphores = &command_buffer_data->swapchain->image_available_semaphore;
		submit_info.pWaitDstStageMask = &wait_stage;
		submit_info.signalSemaphoreCount = 1u;
		submit_info.pSignalSemaphores = &command_buffer_data->swapchain->render_finished_semaphore;
	}

	const VkFence submit_fence = command_buffer_data->submit_fence;
	const VkResult submit_result = vk_expect<vkQueueSubmit>(device_data->direct_queue.queue, 1u, &submit_info, submit_fence);
	command_buffer_data->submitted = submit_result == VK_SUCCESS || submit_result == VK_SUBOPTIMAL_KHR;
}

void vk_device_impl::vk_device_flush(const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	vk_expect<vkQueueWaitIdle>(device_data->direct_queue.queue);
	if (device_data->compute_queue.queue != VK_NULL_HANDLE && device_data->compute_queue.queue != device_data->direct_queue.queue)
		vk_expect<vkQueueWaitIdle>(device_data->compute_queue.queue);
	if (device_data->copy_queue.queue != VK_NULL_HANDLE &&
		device_data->copy_queue.queue != device_data->direct_queue.queue &&
		device_data->copy_queue.queue != device_data->compute_queue.queue)
		vk_expect<vkQueueWaitIdle>(device_data->copy_queue.queue);
}

bool vk_device_impl::vk_device_supports_feature(const device& _device, device_feature feature) {
	if (_device.engine != vulkan_t::get_functions())
		return false;

	auto* data = _device.data.get<vk_device_data>();
	if (data == nullptr)
		return false;

	switch (feature) {
	case device_feature::generated_commands:
		return data->supports_device_generated_commands;
	default:
		return false;
	}
}

void vk_device_impl::vk_device_destroy(device& _device) {
	auto* data = _device.data.expect<vk_device_data>();
	if (data->device != VK_NULL_HANDLE)
		vk_expect<vkDeviceWaitIdle>(data->device);

	if (data->linear_clamp_sampler != VK_NULL_HANDLE)
		vkDestroySampler(data->device, data->linear_clamp_sampler, nullptr);
	if (data->point_clamp_sampler != VK_NULL_HANDLE)
		vkDestroySampler(data->device, data->point_clamp_sampler, nullptr);
	if (data->sampler_descriptor_pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(data->device, data->sampler_descriptor_pool, nullptr);
	if (data->empty_descriptor_pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(data->device, data->empty_descriptor_pool, nullptr);
	if (data->empty_set_layout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(data->device, data->empty_set_layout, nullptr);
	if (data->bindless_pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(data->device, data->bindless_pool, nullptr);
	if (data->bindless_set_layout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(data->device, data->bindless_set_layout, nullptr);

	if (data->device != VK_NULL_HANDLE)
		vkDestroyDevice(data->device, nullptr);

	if (data->debug_messenger != VK_NULL_HANDLE) {
		auto destroy_fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(data->instance, "vkDestroyDebugUtilsMessengerEXT")
		);
		if (destroy_fn)
			destroy_fn(data->instance, data->debug_messenger, nullptr);
	}
	if (data->instance != VK_NULL_HANDLE)
		vkDestroyInstance(data->instance, nullptr);

	delete data;
	_device = {};
}
} // namespace mars::graphics::vk

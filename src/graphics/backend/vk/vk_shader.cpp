#include <mars/graphics/backend/vk/vk_shader.hpp>

#include "vk_internal.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <unknwn.h>
#include <objidl.h>
#include <wrl.h>
#include <wrl/implements.h>
using dxc_module_t = HMODULE;
#else
#include <dlfcn.h>
using dxc_module_t = void*;
#endif

#include <dxcapi.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace mars::graphics::vk {
namespace {
constexpr uint32_t kUniformBindingCount = 16u;
constexpr uint32_t kSampledImageBindingCount = 32u;
constexpr uint32_t kSamplerBindingCount = 4u;
constexpr uint32_t kDescriptorSetIndex = 1u;
constexpr uint32_t kSamplerSetIndex = 3u;

#ifdef _WIN32
using dxc_create_instance_proc = HRESULT(WINAPI*)(REFCLSID, REFIID, LPVOID*);
#define DXC_PTR(T) Microsoft::WRL::ComPtr<T>
#else
using dxc_create_instance_proc = HRESULT(*)(REFCLSID, REFIID, LPVOID*);
template<typename T>
struct dxc_ptr {
	T* p = nullptr;
	~dxc_ptr() { if (p) p->Release(); }
	T** operator&() { return &p; }
	T* Get() const { return p; }
	T* operator->() const { return p; }
	explicit operator bool() const { return p != nullptr; }
	T* Detach() { T* t = p; p = nullptr; return t; }
	dxc_ptr() = default;
	dxc_ptr(const dxc_ptr&) = delete;
	dxc_ptr& operator=(const dxc_ptr&) = delete;
};
#define DXC_PTR(T) dxc_ptr<T>
#endif

std::string read_file_binary(const std::filesystem::path& _path) {
	std::ifstream file(_path, std::ios::binary);
	if (!file.is_open())
		return {};
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

std::string load_shader_source(const shader_module& _module) {
	if (!_module.source.empty())
		return std::string(_module.source);
	if (!_module.path.empty())
		return read_file_binary(std::filesystem::path(_module.path));
	return {};
}

std::string shader_debug_name(const shader_module& _module) {
	if (!_module.name.empty())
		return std::string(_module.name);
	if (!_module.path.empty())
		return std::string(_module.path);
	return "<inline>";
}

vk_sampler_kind detect_sampler_kind(const std::string& _source) {
	if (_source.find("point_sampler") != std::string::npos || _source.find("pointClamp") != std::string::npos)
		return vk_sampler_kind::point_clamp;
	if (_source.find("SamplerState") != std::string::npos)
		return vk_sampler_kind::linear_clamp;
	return vk_sampler_kind::none;
}

std::wstring to_wstring(const std::filesystem::path& _path) {
	return _path.wstring();
}

std::filesystem::path find_vulkan_sdk_dxc() {
#ifdef _WIN32
	std::vector<wchar_t> buffer(32768, L'\0');
	const DWORD length = GetEnvironmentVariableW(L"VULKAN_SDK", buffer.data(), static_cast<DWORD>(buffer.size()));
	if (length > 0 && length < buffer.size()) {
		const std::filesystem::path candidate = std::filesystem::path(buffer.data()) / "Bin" / "dxcompiler.dll";
		if (std::filesystem::exists(candidate))
			return candidate;
	}
	const std::filesystem::path fallback = LR"(C:\VulkanSDK\1.4.341.1\Bin\dxcompiler.dll)";
	if (std::filesystem::exists(fallback))
		return fallback;
#else
	if (const char* sdk = std::getenv("VULKAN_SDK")) {
		const std::filesystem::path candidate = std::filesystem::path(sdk) / "lib" / "libdxcompiler.so";
		if (std::filesystem::exists(candidate))
			return candidate;
	}
	for (const char* p : {"/usr/lib/libdxcompiler.so", "/usr/local/lib/libdxcompiler.so"})
		if (std::filesystem::exists(p))
			return p;
#endif
	return {};
}

dxc_module_t load_dxc_module() {
	static dxc_module_t module = []() -> dxc_module_t {
		if (const std::filesystem::path preferred_path = find_vulkan_sdk_dxc(); !preferred_path.empty()) {
#ifdef _WIN32
			if (HMODULE handle = LoadLibraryW(preferred_path.c_str()))
				return handle;
#else
			if (void* handle = dlopen(preferred_path.c_str(), RTLD_LAZY))
				return handle;
#endif
		}
#ifdef _WIN32
		if (HMODULE handle = GetModuleHandleW(L"dxcompiler.dll"))
			return handle;
		return LoadLibraryW(L"dxcompiler.dll");
#else
		return dlopen("libdxcompiler.so", RTLD_LAZY);
#endif
	}();
	return module;
}

HRESULT vk_dxc_create_instance(REFCLSID _clsid, REFIID _iid, void** _out_object) {
	if (dxc_module_t module = load_dxc_module()) {
		const auto proc = reinterpret_cast<dxc_create_instance_proc>(
#ifdef _WIN32
			GetProcAddress(module, "DxcCreateInstance")
#else
			dlsym(module, "DxcCreateInstance")
#endif
		);
		if (proc)
			return proc(_clsid, _iid, _out_object);
	}
	return E_FAIL;
}

void append_bindings(std::vector<std::wstring>& _storage) {
	for (uint32_t binding = 0u; binding < kUniformBindingCount; ++binding) {
		_storage.push_back(L"-fvk-bind-register");
		_storage.push_back(std::wstring(L"b") + std::to_wstring(binding));
		_storage.push_back(L"0");
		_storage.push_back(std::to_wstring(binding));
		_storage.push_back(std::to_wstring(kDescriptorSetIndex));
	}

	_storage.push_back(L"-fvk-bind-register");
	_storage.push_back(L"b0");
	_storage.push_back(L"1");
	_storage.push_back(L"0");
	_storage.push_back(std::to_wstring(kDescriptorSetIndex));

	for (uint32_t binding = 0u; binding < kSampledImageBindingCount; ++binding) {
		_storage.push_back(L"-fvk-bind-register");
		_storage.push_back(std::wstring(L"t") + std::to_wstring(binding));
		_storage.push_back(L"0");
		_storage.push_back(std::to_wstring(binding));
		_storage.push_back(std::to_wstring(kDescriptorSetIndex));
	}

	for (uint32_t binding = 0u; binding < kSamplerBindingCount; ++binding) {
		_storage.push_back(L"-fvk-bind-register");
		_storage.push_back(std::wstring(L"s") + std::to_wstring(binding));
		_storage.push_back(L"0");
		_storage.push_back(std::to_wstring(binding));
		_storage.push_back(std::to_wstring(kSamplerSetIndex));
	}
}

bool is_rt_shader(mars_shader_type _shader_type) {
	return _shader_type == MARS_SHADER_TYPE_RAY_GEN || _shader_type == MARS_SHADER_TYPE_MISS || _shader_type == MARS_SHADER_TYPE_CLOSEST_HIT;
}

bool compile_module(vk_device_data* _device_data, IDxcUtils*, IDxcCompiler3* _compiler, IDxcIncludeHandler* _include_handler, const shader_module& _module, const wchar_t* _entry_point, const wchar_t* _profile, mars_shader_type _shader_type, vk_shader_stage_data* _out_stage) {
	const std::string source = load_shader_source(_module);
	if (source.empty()) {
		mars::logger::error(vk_log_channel(), "Unable to load shader source {}", shader_debug_name(_module));
		return false;
	}

	const std::filesystem::path absolute_path = _module.path.empty()
		? std::filesystem::current_path()
		: std::filesystem::absolute(std::filesystem::path(_module.path));
	std::vector<std::wstring> arg_storage = {
		L"-spirv",
		L"-E",
		_entry_point,
		L"-T",
		_profile,
		L"-fspv-target-env=vulkan1.3",
		L"-fvk-use-dx-layout",
		L"-fspv-extension=SPV_EXT_descriptor_indexing",
		L"-fvk-bind-resource-heap",
		L"0",
		L"0",
		L"-I",
		to_wstring(absolute_path.parent_path()),
	};
	if (is_rt_shader(_shader_type))
		arg_storage.push_back(L"-fspv-extension=SPV_KHR_ray_tracing");
	append_bindings(arg_storage);

	std::vector<LPCWSTR> args;
	args.reserve(arg_storage.size());
	for (const auto& arg : arg_storage)
		args.push_back(arg.c_str());

	DxcBuffer buffer = {};
	buffer.Ptr = source.data();
	buffer.Size = source.size();
	buffer.Encoding = DXC_CP_UTF8;

	DXC_PTR(IDxcResult) result;
	const HRESULT compile_hr = _compiler->Compile(&buffer, args.data(), static_cast<UINT32>(args.size()), _include_handler, IID_PPV_ARGS(&result));
	if (FAILED(compile_hr)) {
		mars::logger::error(vk_log_channel(), "DXC compile failed for {}", shader_debug_name(_module));
		return false;
	}

	HRESULT status = S_OK;
	result->GetStatus(&status);

	DXC_PTR(IDxcBlobUtf8) errors;
	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	if (errors && errors->GetStringLength() > 0) {
		if (FAILED(status))
			mars::logger::error(vk_log_channel(), "{}", errors->GetStringPointer());
		else
			mars::logger::warning(vk_log_channel(), "{}", errors->GetStringPointer());
	}
	if (FAILED(status))
		return false;

	DXC_PTR(IDxcBlob) blob;
	result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&blob), nullptr);

	VkShaderModuleCreateInfo module_info = {};
	module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_info.codeSize = blob->GetBufferSize();
	module_info.pCode = static_cast<const uint32_t*>(blob->GetBufferPointer());
	vk_expect<vkCreateShaderModule>(_device_data->device, &module_info, nullptr, &_out_stage->module);

	_out_stage->path = shader_debug_name(_module);
	_out_stage->sampler_kind = detect_sampler_kind(source);
	return _out_stage->module != VK_NULL_HANDLE;
}

std::filesystem::path resolve_vulkan_shader_path(const std::filesystem::path& _original_path) {
	if (_original_path.empty())
		return _original_path;

	const std::string normalized = _original_path.lexically_normal().generic_string();
	if (normalized.empty())
		return _original_path;

	if (normalized.rfind("fx/vk/", 0u) == 0u || normalized.find("/fx/vk/") != std::string::npos)
		return _original_path;

	std::string vk_variant = normalized;
	const size_t rooted_fx_offset = vk_variant.find("/fx/");
	if (rooted_fx_offset != std::string::npos)
		vk_variant.insert(rooted_fx_offset + 4u, "vk/");
	else if (vk_variant.rfind("fx/", 0u) == 0u)
		vk_variant.insert(3u, "vk/");
	else
		return _original_path;

	const std::filesystem::path variant_path(vk_variant);
	if (std::filesystem::exists(variant_path))
		return variant_path;

	return _original_path;
}
} // namespace

shader vk_shader_impl::vk_shader_create(const device& _device, const std::vector<shader_module>& _shaders) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_shader_data();

	DXC_PTR(IDxcUtils) dxc_utils;
	DXC_PTR(IDxcCompiler3) dxc_compiler;
	DXC_PTR(IDxcIncludeHandler) default_include_handler;
	if (FAILED(vk_dxc_create_instance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils))) ||
		FAILED(vk_dxc_create_instance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler))) ||
		!dxc_utils ||
		!dxc_compiler ||
		FAILED(dxc_utils->CreateDefaultIncludeHandler(&default_include_handler))) {
		mars::logger::error(vk_log_channel(), "Failed to initialize DXC compiler interfaces");

		shader result;
		result.engine = _device.engine;
		result.data.store(data);
		return result;
	}

	for (const auto& module : _shaders) {
		shader_module resolved_module = module;
		std::string resolved_path_storage;
		if (resolved_module.source.empty() && !resolved_module.path.empty()) {
			const std::filesystem::path original_path(module.path);
			const std::filesystem::path path = resolve_vulkan_shader_path(original_path);
			resolved_path_storage = path.string();
			resolved_module.path = resolved_path_storage;
		}
		switch (module.type) {
		case MARS_SHADER_TYPE_VERTEX:
			compile_module(device_data, dxc_utils.Get(), dxc_compiler.Get(), default_include_handler.Get(), resolved_module, L"VSMain", L"vs_6_8", module.type, &data->vertex);
			break;
		case MARS_SHADER_TYPE_FRAGMENT:
			compile_module(device_data, dxc_utils.Get(), dxc_compiler.Get(), default_include_handler.Get(), resolved_module, L"PSMain", L"ps_6_6", module.type, &data->fragment);
			break;
		case MARS_SHADER_TYPE_COMPUTE:
			compile_module(device_data, dxc_utils.Get(), dxc_compiler.Get(), default_include_handler.Get(), resolved_module, L"CSMain", L"cs_6_6", module.type, &data->compute);
			break;
		case MARS_SHADER_TYPE_RAY_GEN:
			compile_module(device_data, dxc_utils.Get(), dxc_compiler.Get(), default_include_handler.Get(), resolved_module, L"RayGen", L"lib_6_6", module.type, &data->compute);
			break;
		case MARS_SHADER_TYPE_MISS:
			compile_module(device_data, dxc_utils.Get(), dxc_compiler.Get(), default_include_handler.Get(), resolved_module, L"Miss", L"lib_6_6", module.type, &data->compute);
			break;
		case MARS_SHADER_TYPE_CLOSEST_HIT:
			compile_module(device_data, dxc_utils.Get(), dxc_compiler.Get(), default_include_handler.Get(), resolved_module, L"ClosestHit", L"lib_6_6", module.type, &data->compute);
			break;
		}
	}

	shader result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

void vk_shader_impl::vk_shader_destroy(shader& _shader, const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = _shader.data.expect<vk_shader_data>();

	if (data->vertex.module != VK_NULL_HANDLE)
		vkDestroyShaderModule(device_data->device, data->vertex.module, nullptr);
	if (data->fragment.module != VK_NULL_HANDLE)
		vkDestroyShaderModule(device_data->device, data->fragment.module, nullptr);
	if (data->compute.module != VK_NULL_HANDLE)
		vkDestroyShaderModule(device_data->device, data->compute.module, nullptr);

	delete data;
	_shader = {};
}
} // namespace mars::graphics::vk

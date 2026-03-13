#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_shader.hpp>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <unknwn.h>

#include <objidl.h>

#include <dxcapi.h>
#include <wrl.h>
#include <wrl/implements.h>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace mars::graphics::vk {
namespace {
using dxc_create_instance_proc = HRESULT(WINAPI*)(REFCLSID, REFIID, LPVOID*);

std::string read_file_binary(const std::filesystem::path& path) {
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
		return {};
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

vk_sampler_kind detect_sampler_kind(const std::string& source) {
	if (source.find("point_sampler") != std::string::npos || source.find("pointClamp") != std::string::npos)
		return vk_sampler_kind::point_clamp;
	if (source.find("SamplerState") != std::string::npos)
		return vk_sampler_kind::linear_clamp;
	return vk_sampler_kind::none;
}

std::wstring to_wstring(const std::filesystem::path& path) {
	return path.wstring();
}

std::filesystem::path find_vulkan_sdk_dxc() {
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

	return {};
}

HMODULE load_dxc_module() {
	static HMODULE module = []() -> HMODULE {
		if (const std::filesystem::path preferred = find_vulkan_sdk_dxc(); !preferred.empty()) {
			if (HMODULE handle = LoadLibraryW(preferred.c_str()))
				return handle;
		}

		if (HMODULE handle = GetModuleHandleW(L"dxcompiler.dll"))
			return handle;

		return LoadLibraryW(L"dxcompiler.dll");
	}();

	return module;
}

HRESULT vk_dxc_create_instance(REFCLSID clsid, REFIID iid, void** out_object) {
	if (HMODULE module = load_dxc_module()) {
		if (auto proc = reinterpret_cast<dxc_create_instance_proc>(GetProcAddress(module, "DxcCreateInstance")))
			return proc(clsid, iid, out_object);
	}

	return E_FAIL;
}

std::string trim_copy(std::string_view value) {
	size_t begin = 0u;
	size_t end = value.size();
	while (begin < end && std::isspace(static_cast<unsigned char>(value[begin])) != 0)
		++begin;
	while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1u])) != 0)
		--end;
	return std::string(value.substr(begin, end - begin));
}

std::string remove_comments(std::string_view source) {
	std::string result;
	result.reserve(source.size());

	bool in_line_comment = false;
	bool in_block_comment = false;
	for (size_t index = 0u; index < source.size(); ++index) {
		const char current = source[index];
		const char next = index + 1u < source.size() ? source[index + 1u] : '\0';

		if (in_line_comment) {
			if (current == '\n') {
				in_line_comment = false;
				result.push_back(current);
			}
			continue;
		}

		if (in_block_comment) {
			if (current == '*' && next == '/') {
				in_block_comment = false;
				++index;
			}
			continue;
		}

		if (current == '/' && next == '/') {
			in_line_comment = true;
			++index;
			continue;
		}
		if (current == '/' && next == '*') {
			in_block_comment = true;
			++index;
			continue;
		}

		result.push_back(current);
	}

	return result;
}

std::vector<std::string> extract_push_constant_members(const std::string& body) {
	const std::string cleaned_body = remove_comments(body);

	std::vector<std::string> members;
	size_t segment_begin = 0u;
	while (segment_begin < cleaned_body.size()) {
		const size_t segment_end = cleaned_body.find(';', segment_begin);
		if (segment_end == std::string::npos)
			break;

		const std::string segment = trim_copy(std::string_view(cleaned_body).substr(segment_begin, segment_end - segment_begin));
		segment_begin = segment_end + 1u;
		if (segment.empty())
			continue;

		size_t cursor = segment.find_last_not_of(" \t\r\n");
		if (cursor == std::string::npos)
			continue;
		if (segment[cursor] == ']') {
			const size_t array_begin = segment.find_last_of('[', cursor);
			if (array_begin == std::string::npos)
				continue;
			cursor = array_begin == 0u ? std::string::npos : array_begin - 1u;
			while (cursor != std::string::npos && std::isspace(static_cast<unsigned char>(segment[cursor])) != 0)
				cursor = cursor == 0u ? std::string::npos : cursor - 1u;
			if (cursor == std::string::npos)
				continue;
		}

		size_t name_end = cursor + 1u;
		size_t name_begin = cursor;
		while (name_begin > 0u) {
			const char ch = segment[name_begin - 1u];
			if (std::isalnum(static_cast<unsigned char>(ch)) == 0 && ch != '_')
				break;
			--name_begin;
		}
		if (name_begin == name_end)
			continue;
		members.push_back(segment.substr(name_begin, name_end - name_begin));
	}

	return members;
}

bool is_identifier_char(char ch) {
	return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

size_t skip_whitespace(std::string_view source, size_t offset) {
	while (offset < source.size() && std::isspace(static_cast<unsigned char>(source[offset])) != 0)
		++offset;
	return offset;
}

bool match_keyword(std::string_view source, size_t offset, std::string_view keyword) {
	if (offset + keyword.size() > source.size())
		return false;
	if (source.substr(offset, keyword.size()) != keyword)
		return false;
	if (offset > 0u && is_identifier_char(source[offset - 1u]))
		return false;
	if (offset + keyword.size() < source.size() && is_identifier_char(source[offset + keyword.size()]))
		return false;
	return true;
}

bool parse_identifier(std::string_view source, size_t& offset, std::string& out_identifier) {
	if (offset >= source.size())
		return false;
	if (std::isalpha(static_cast<unsigned char>(source[offset])) == 0 && source[offset] != '_')
		return false;

	const size_t begin = offset;
	++offset;
	while (offset < source.size() && is_identifier_char(source[offset]))
		++offset;

	out_identifier.assign(source.substr(begin, offset - begin));
	return true;
}

size_t find_matching_brace(std::string_view source, size_t open_brace_offset) {
	bool in_line_comment = false;
	bool in_block_comment = false;
	bool in_string = false;

	for (size_t offset = open_brace_offset + 1u; offset < source.size(); ++offset) {
		const char current = source[offset];
		const char next = offset + 1u < source.size() ? source[offset + 1u] : '\0';

		if (in_line_comment) {
			if (current == '\n')
				in_line_comment = false;
			continue;
		}

		if (in_block_comment) {
			if (current == '*' && next == '/') {
				in_block_comment = false;
				++offset;
			}
			continue;
		}

		if (in_string) {
			if (current == '\\') {
				++offset;
				continue;
			}
			if (current == '"')
				in_string = false;
			continue;
		}

		if (current == '/' && next == '/') {
			in_line_comment = true;
			++offset;
			continue;
		}
		if (current == '/' && next == '*') {
			in_block_comment = true;
			++offset;
			continue;
		}
		if (current == '"') {
			in_string = true;
			continue;
		}
		if (current == '}')
			return offset;
	}

	return std::string_view::npos;
}

std::string transform_push_constant_cbuffers(const std::string& source) {
	std::string transformed;
	transformed.reserve(source.size() + 512u);

	size_t scan_offset = 0u;
	size_t copy_offset = 0u;
	while (scan_offset < source.size()) {
		const size_t cbuffer_offset = source.find("cbuffer", scan_offset);
		if (cbuffer_offset == std::string::npos)
			break;
		if (!match_keyword(source, cbuffer_offset, "cbuffer")) {
			scan_offset = cbuffer_offset + 1u;
			continue;
		}

		size_t parse_offset = cbuffer_offset + 7u;
		parse_offset = skip_whitespace(source, parse_offset);

		std::string block_name;
		if (!parse_identifier(source, parse_offset, block_name)) {
			scan_offset = cbuffer_offset + 7u;
			continue;
		}

		parse_offset = skip_whitespace(source, parse_offset);
		if (parse_offset >= source.size() || source[parse_offset] != ':') {
			scan_offset = cbuffer_offset + 7u;
			continue;
		}
		++parse_offset;
		parse_offset = skip_whitespace(source, parse_offset);

		if (!match_keyword(source, parse_offset, "register")) {
			scan_offset = cbuffer_offset + 7u;
			continue;
		}
		parse_offset += 8u;
		parse_offset = skip_whitespace(source, parse_offset);

		if (parse_offset >= source.size() || source[parse_offset] != '(') {
			scan_offset = cbuffer_offset + 7u;
			continue;
		}
		++parse_offset;
		parse_offset = skip_whitespace(source, parse_offset);

		if (parse_offset + 3u > source.size() || source.substr(parse_offset, 3u) != "b15") {
			scan_offset = cbuffer_offset + 7u;
			continue;
		}
		parse_offset += 3u;
		parse_offset = skip_whitespace(source, parse_offset);

		if (parse_offset >= source.size() || source[parse_offset] != ')') {
			scan_offset = cbuffer_offset + 7u;
			continue;
		}
		++parse_offset;
		parse_offset = skip_whitespace(source, parse_offset);

		if (parse_offset >= source.size() || source[parse_offset] != '{') {
			scan_offset = cbuffer_offset + 7u;
			continue;
		}

		const size_t body_begin = parse_offset + 1u;
		const size_t body_end = find_matching_brace(source, parse_offset);
		if (body_end == std::string::npos) {
			scan_offset = cbuffer_offset + 7u;
			continue;
		}

		size_t end_offset = body_end + 1u;
		end_offset = skip_whitespace(source, end_offset);
		if (end_offset >= source.size() || source[end_offset] != ';') {
			scan_offset = cbuffer_offset + 7u;
			continue;
		}
		++end_offset;

		transformed.append(source, copy_offset, cbuffer_offset - copy_offset);

		const std::string body = source.substr(body_begin, body_end - body_begin);
		const std::vector<std::string> members = extract_push_constant_members(body);
		const std::string type_name = block_name + "_vk_push_constant_type";
		const std::string variable_name = block_name + "_vk_push_constant";

		transformed += "struct " + type_name + " {";
		transformed += body;
		transformed += "};\n";
		transformed += "[[vk::push_constant]] ConstantBuffer<" + type_name + "> " + variable_name + ";\n";
		for (const auto& member : members)
			transformed += "#define " + member + " (" + variable_name + "." + member + ")\n";

		copy_offset = end_offset;
		scan_offset = end_offset;
	}

	transformed.append(source, copy_offset, std::string::npos);
	return transformed;
}

void append_bindings(std::vector<std::wstring>& storage) {
	for (uint32_t binding = 0u; binding < 15u; ++binding) {
		storage.push_back(L"-fvk-bind-register");
		storage.push_back(std::wstring(L"b") + std::to_wstring(binding));
		storage.push_back(L"0");
		storage.push_back(std::to_wstring(binding));
		storage.push_back(L"1");
	}

	for (uint32_t binding = 0u; binding < 4u; ++binding) {
		storage.push_back(L"-fvk-bind-register");
		storage.push_back(std::wstring(L"s") + std::to_wstring(binding));
		storage.push_back(L"0");
		storage.push_back(std::to_wstring(binding));
		storage.push_back(L"3");
	}
}

class transformed_include_handler final : public Microsoft::WRL::RuntimeClass<
											  Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
											  IDxcIncludeHandler> {
  public:
	HRESULT RuntimeClassInitialize(IDxcUtils* utils, IDxcIncludeHandler* fallback) {
		m_utils = utils;
		m_fallback = fallback;
		return (m_utils && m_fallback) ? S_OK : E_INVALIDARG;
	}

	HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR filename, IDxcBlob** include_source) override {
		if (!include_source)
			return E_POINTER;

		Microsoft::WRL::ComPtr<IDxcBlob> blob;
		const HRESULT load_hr = m_fallback->LoadSource(filename, &blob);
		if (FAILED(load_hr) || !blob)
			return load_hr;

		const auto* blob_bytes = static_cast<const char*>(blob->GetBufferPointer());
		const std::string source(blob_bytes, blob->GetBufferSize());
		const std::string transformed = transform_push_constant_cbuffers(source);

		Microsoft::WRL::ComPtr<IDxcBlobEncoding> encoding_blob;
		const HRESULT create_hr = m_utils->CreateBlob(
			transformed.data(),
			static_cast<UINT32>(transformed.size()),
			DXC_CP_UTF8,
			&encoding_blob
		);
		if (FAILED(create_hr))
			return create_hr;

		*include_source = encoding_blob.Detach();
		return S_OK;
	}

  private:
	Microsoft::WRL::ComPtr<IDxcUtils> m_utils;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> m_fallback;
};

bool compile_module(
	vk_device_data* device_data,
	IDxcUtils* dxc_utils,
	IDxcCompiler3* compiler,
	IDxcIncludeHandler* include_handler,
	const std::filesystem::path& path,
	const wchar_t* entry_point,
	const wchar_t* profile,
	vk_shader_stage_data* out_stage
) {
	const std::string source = read_file_binary(path);
	if (source.empty()) {
		mars::logger::error(vk_log_channel(), "Unable to read shader file {}", path.string());
		return false;
	}

	const std::string transformed_source = transform_push_constant_cbuffers(source);
	const std::filesystem::path absolute_path = std::filesystem::absolute(path);
	std::vector<std::wstring> arg_storage = {
		L"-spirv",
		L"-E",
		entry_point,
		L"-T",
		profile,
		L"-fspv-target-env=vulkan1.3",
		L"-fvk-use-dx-layout",
		L"-fspv-extension=SPV_EXT_descriptor_indexing",
		L"-fvk-bind-resource-heap",
		L"0",
		L"0",
		L"-I",
		to_wstring(absolute_path.parent_path()),
	};
	append_bindings(arg_storage);

	std::vector<LPCWSTR> args;
	args.reserve(arg_storage.size());
	for (const auto& arg : arg_storage)
		args.push_back(arg.c_str());

	DxcBuffer buffer = {};
	buffer.Ptr = transformed_source.data();
	buffer.Size = transformed_source.size();
	buffer.Encoding = DXC_CP_UTF8;

	Microsoft::WRL::ComPtr<IDxcResult> result;
	const HRESULT compile_hr = compiler->Compile(&buffer, args.data(), static_cast<UINT32>(args.size()), include_handler, IID_PPV_ARGS(&result));
	if (FAILED(compile_hr)) {
		mars::logger::error(vk_log_channel(), "DXC compile failed for {}", path.string());
		return false;
	}

	HRESULT status = S_OK;
	result->GetStatus(&status);

	Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	if (errors && errors->GetStringLength() > 0) {
		if (FAILED(status))
			mars::logger::error(vk_log_channel(), "{}", errors->GetStringPointer());
		else
			mars::logger::warning(vk_log_channel(), "{}", errors->GetStringPointer());
	}
	if (FAILED(status))
		return false;

	Microsoft::WRL::ComPtr<IDxcBlob> blob;
	result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&blob), nullptr);

	VkShaderModuleCreateInfo module_info = {};
	module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_info.codeSize = blob->GetBufferSize();
	module_info.pCode = static_cast<const uint32_t*>(blob->GetBufferPointer());
	vk_expect<vkCreateShaderModule>(device_data->device, &module_info, nullptr, &out_stage->module);

	out_stage->path = path.string();
	out_stage->sampler_kind = detect_sampler_kind(source);
	return out_stage->module != VK_NULL_HANDLE;
}

std::filesystem::path resolve_vulkan_shader_path(const std::filesystem::path& original_path) {
	if (original_path.empty())
		return original_path;

	const std::string normalized = original_path.lexically_normal().generic_string();
	if (normalized.empty())
		return original_path;

	if (normalized.rfind("fx/vk/", 0u) == 0u || normalized.find("/fx/vk/") != std::string::npos)
		return original_path;

	std::string vk_variant = normalized;
	const size_t rooted_fx_offset = vk_variant.find("/fx/");
	if (rooted_fx_offset != std::string::npos)
		vk_variant.insert(rooted_fx_offset + 4u, "vk/");
	else if (vk_variant.rfind("fx/", 0u) == 0u)
		vk_variant.insert(3u, "vk/");
	else
		return original_path;

	const std::filesystem::path variant_path(vk_variant);
	if (std::filesystem::exists(variant_path))
		return variant_path;

	return original_path;
}
} // namespace

shader vk_shader_impl::vk_shader_create(const device& _device, const std::vector<shader_module>& _shaders) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_shader_data();

	Microsoft::WRL::ComPtr<IDxcUtils> dxc_utils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxc_compiler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> default_include_handler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> transformed_handler;
	if (FAILED(vk_dxc_create_instance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils))) ||
		FAILED(vk_dxc_create_instance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler))) ||
		!dxc_utils ||
		!dxc_compiler ||
		FAILED(dxc_utils->CreateDefaultIncludeHandler(&default_include_handler)) ||
		FAILED(Microsoft::WRL::MakeAndInitialize<transformed_include_handler>(&transformed_handler, dxc_utils.Get(), default_include_handler.Get()))) {
		mars::logger::error(vk_log_channel(), "Failed to initialize DXC compiler interfaces");

		shader result;
		result.engine = _device.engine;
		result.data.store(data);
		return result;
	}

	for (const auto& module : _shaders) {
		const std::filesystem::path original_path(module.path);
		const std::filesystem::path path = resolve_vulkan_shader_path(original_path);
		switch (module.type) {
		case MARS_SHADER_TYPE_VERTEX:
			compile_module(device_data, dxc_utils.Get(), dxc_compiler.Get(), transformed_handler.Get(), path, L"VSMain", L"vs_6_8", &data->vertex);
			break;
		case MARS_SHADER_TYPE_FRAGMENT:
			compile_module(device_data, dxc_utils.Get(), dxc_compiler.Get(), transformed_handler.Get(), path, L"PSMain", L"ps_6_6", &data->fragment);
			break;
		case MARS_SHADER_TYPE_COMPUTE:
			compile_module(device_data, dxc_utils.Get(), dxc_compiler.Get(), transformed_handler.Get(), path, L"CSMain", L"cs_6_6", &data->compute);
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

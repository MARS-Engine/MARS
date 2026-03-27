#include <mars/graphics/backend/dx12/dx_shader.hpp>

#include "dx_internal.hpp"
#include <mars/graphics/functional/device.hpp>

#include <mars/debug/logger.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace mars::graphics::dx {
static log_channel g_dx12_channel("dx12");

static std::string read_file(std::string_view _path) {
	std::ifstream file{std::string{_path}};
	if (!file.is_open())
		return "";
	std::stringstream buf;
	buf << file.rdbuf();
	return buf.str();
}

static std::string load_shader_source(const shader_module& _module) {
	if (!_module.source.empty())
		return std::string(_module.source);
	return read_file(_module.path);
}

static std::string shader_debug_name(const shader_module& _module) {
	if (!_module.name.empty())
		return std::string(_module.name);
	if (!_module.path.empty())
		return std::string(_module.path);
	return "<inline>";
}

static HRESULT compile_shader_dxc(IDxcCompiler3* _compiler, IDxcUtils*, IDxcIncludeHandler* _handler, const std::string& _source, const wchar_t* _entry, const wchar_t* _profile, ID3DBlob** _out_blob) {
	DxcBuffer src_buf = {};
	src_buf.Ptr = _source.c_str();
	src_buf.Size = _source.length();
	src_buf.Encoding = DXC_CP_UTF8;

	std::vector<LPCWSTR> args = {L"-E", _entry, L"-T", _profile, L"-Zi", L"-Qembed_debug", L"-Od"};

	Microsoft::WRL::ComPtr<IDxcResult> result;
	if (!dx_expect<&IDxcCompiler3::Compile>(_compiler, &src_buf, args.data(), static_cast<UINT32>(args.size()), _handler, IID_PPV_ARGS(&result)))
		return E_FAIL;

	HRESULT compile_status = S_OK;
	if (!dx_expect<&IDxcResult::GetStatus>(result.Get(), &compile_status))
		return E_FAIL;

	Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
	if (!dx_expect<&IDxcResult::GetOutput>(result.Get(), DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr))
		return E_FAIL;
	if (FAILED(compile_status)) {
		if (errors && errors->GetStringLength() > 0)
			logger::error(g_dx12_channel, "DXC compilation error: {}", errors->GetStringPointer());
		return compile_status;
	}

	if (errors && errors->GetStringLength() > 0)
		logger::warning(g_dx12_channel, "DXC compilation warning: {}", errors->GetStringPointer());

	Microsoft::WRL::ComPtr<IDxcBlob> blob;
	if (!dx_expect<&IDxcResult::GetOutput>(result.Get(), DXC_OUT_OBJECT, IID_PPV_ARGS(&blob), nullptr))
		return E_FAIL;

	*_out_blob = reinterpret_cast<ID3DBlob*>(blob.Detach());
	return S_OK;
}

shader dx_shader_impl::dx_shader_create(const device& _device, const std::vector<shader_module>& _shaders) {
	auto* shader_data = new dx_shader_data();

	Microsoft::WRL::ComPtr<IDxcUtils> dxc_utils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxc_compiler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> dxc_handler;
	if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils)))) {
		logger::error(g_dx12_channel, "DxcCreateInstance failed for IDxcUtils");
		delete shader_data;
		return {};
	}
	if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler)))) {
		logger::error(g_dx12_channel, "DxcCreateInstance failed for IDxcCompiler3");
		delete shader_data;
		return {};
	}
	if (!dx_expect<&IDxcUtils::CreateDefaultIncludeHandler>(dxc_utils.Get(), &dxc_handler)) {
		delete shader_data;
		return {};
	}

	for (const auto& module : _shaders) {
		std::string source = load_shader_source(module);
		if (source.empty()) {
			logger::error(g_dx12_channel, "Unable to load shader source: {}", shader_debug_name(module));
			continue;
		}

		HRESULT compile_hr = S_OK;
		const std::string debug_name = shader_debug_name(module);
		if (module.type == MARS_SHADER_TYPE_VERTEX) {
			compile_hr = compile_shader_dxc(dxc_compiler.Get(), dxc_utils.Get(), dxc_handler.Get(), source, L"VSMain", L"vs_6_6", &shader_data->vertex_shader);
			shader_data->vertex_shader_path = debug_name;
		}
		else if (module.type == MARS_SHADER_TYPE_FRAGMENT) {
			compile_hr = compile_shader_dxc(dxc_compiler.Get(), dxc_utils.Get(), dxc_handler.Get(), source, L"PSMain", L"ps_6_6", &shader_data->pixel_shader);
			shader_data->pixel_shader_path = debug_name;
		}
		else if (module.type == MARS_SHADER_TYPE_COMPUTE) {
			compile_hr = compile_shader_dxc(dxc_compiler.Get(), dxc_utils.Get(), dxc_handler.Get(), source, L"CSMain", L"cs_6_6", &shader_data->compute_shader);
			shader_data->compute_shader_path = debug_name;
		}
		else if (module.type == MARS_SHADER_TYPE_RAY_GEN) {
			compile_hr = compile_shader_dxc(dxc_compiler.Get(), dxc_utils.Get(), dxc_handler.Get(), source, L"RayGen", L"lib_6_6", &shader_data->compute_shader);
			shader_data->compute_shader_path = debug_name;
		}
		else if (module.type == MARS_SHADER_TYPE_MISS) {
			compile_hr = compile_shader_dxc(dxc_compiler.Get(), dxc_utils.Get(), dxc_handler.Get(), source, L"Miss", L"lib_6_6", &shader_data->compute_shader);
			shader_data->compute_shader_path = debug_name;
		}
		else if (module.type == MARS_SHADER_TYPE_CLOSEST_HIT) {
			compile_hr = compile_shader_dxc(dxc_compiler.Get(), dxc_utils.Get(), dxc_handler.Get(), source, L"ClosestHit", L"lib_6_6", &shader_data->compute_shader);
			shader_data->compute_shader_path = debug_name;
		}

		if (FAILED(compile_hr))
			logger::error(g_dx12_channel, "Shader compile failed for {} type={} hr={:#x}", debug_name, static_cast<int>(module.type), static_cast<unsigned long>(compile_hr));
	}

	shader result = {};
	result.engine = _device.engine;
	result.data.store(shader_data);
	return result;
}

void dx_shader_impl::dx_shader_destroy(shader& _shader, const device&) {
	auto* shader_data = _shader.data.expect<dx_shader_data>();
	delete shader_data;
	_shader = {};
}
} // namespace mars::graphics::dx

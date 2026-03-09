#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_shader.hpp>
#include <mars/graphics/functional/device.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel dx12_channel("dx12");

static std::string read_file(const std::string_view& path) {
	std::ifstream file{std::string{path}};
	if (!file.is_open())
		return "";
	std::stringstream buf;
	buf << file.rdbuf();
	return buf.str();
}

static HRESULT compile_shader_dxc(
    IDxcCompiler3* compiler, IDxcUtils* utils, IDxcIncludeHandler* handler,
    const std::string& source, const wchar_t* entry, const wchar_t* profile,
    ID3DBlob** out_blob) {
	DxcBuffer src_buf;
	src_buf.Ptr = source.c_str();
	src_buf.Size = source.length();
	src_buf.Encoding = DXC_CP_ACP;

	std::vector<LPCWSTR> args = {L"-E", entry, L"-T", profile, L"-Zi", L"-Qembed_debug", L"-Od"};

	Microsoft::WRL::ComPtr<IDxcResult> result;
	HRESULT hr = compiler->Compile(&src_buf, args.data(), (UINT32)args.size(), handler, IID_PPV_ARGS(&result));
	if (FAILED(hr))
		return hr;

	HRESULT compile_status = S_OK;
	result->GetStatus(&compile_status);

	Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	if (FAILED(compile_status)) {
		if (errors && errors->GetStringLength() > 0)
			logger::error(dx12_channel, "DXC compilation error: {}", errors->GetStringPointer());
		return compile_status;
	}

	if (errors && errors->GetStringLength() > 0)
		logger::warning(dx12_channel, "DXC compilation warning: {}", errors->GetStringPointer());

	Microsoft::WRL::ComPtr<IDxcBlob> blob;
	hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&blob), nullptr);
	if (FAILED(hr))
		return hr;

	*out_blob = reinterpret_cast<ID3DBlob*>(blob.Detach());
	return S_OK;
}

shader dx_shader_impl::dx_shader_create(const device& _device, const std::vector<shader_module>& _shaders) {
	auto data = new dx_shader_data();

	Microsoft::WRL::ComPtr<IDxcUtils> dxc_utils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxc_compiler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> dxc_handler;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils));
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler));
	dxc_utils->CreateDefaultIncludeHandler(&dxc_handler);

	for (auto& mod : _shaders) {
		std::string source = read_file(mod.path);
		if (source.empty()) {
			logger::error(dx12_channel, "Unable to read shader file: {}", mod.path);
			continue;
		}

		HRESULT compile_hr = S_OK;
		if (mod.type == MARS_SHADER_TYPE_VERTEX) {
			compile_hr = compile_shader_dxc(dxc_compiler.Get(), dxc_utils.Get(), dxc_handler.Get(),
							source, L"VSMain", L"vs_6_6", &data->vertex_shader);
			data->vertex_shader_path = mod.path;
		} else if (mod.type == MARS_SHADER_TYPE_FRAGMENT) {
			compile_hr = compile_shader_dxc(dxc_compiler.Get(), dxc_utils.Get(), dxc_handler.Get(),
							source, L"PSMain", L"ps_6_6", &data->pixel_shader);
			data->pixel_shader_path = mod.path;
		} else if (mod.type == MARS_SHADER_TYPE_COMPUTE) {
			compile_hr = compile_shader_dxc(dxc_compiler.Get(), dxc_utils.Get(), dxc_handler.Get(),
							source, L"CSMain", L"cs_6_6", &data->compute_shader);
			data->compute_shader_path = mod.path;
		}

		if (FAILED(compile_hr))
			logger::error(dx12_channel, "Shader compile failed for path={} type={} hr={:#x}", mod.path, (int)mod.type, (unsigned long)compile_hr);
	}

	shader result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

void dx_shader_impl::dx_shader_destroy(shader& _shader, const device& _device) {
	auto data = dx_expect_backend_data(_shader.data.get<dx_shader_data>(), __func__, "shader.data");
	delete data;
	_shader = {};
}
} // namespace mars::graphics::dx

#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_swapchain.hpp>
#include <mars/graphics/functional/device.hpp>
#include <mars/graphics/functional/window.hpp>

#include <SDL3/SDL.h>
#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel dx12_channel("dx12");

swapchain dx_swapchain_impl::dx_swapchain_create(const device& _device, const window& _window, const swapchain_create_params& _params) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto cq_data = device_data->command_queue_data.expect<dx_command_queue_data>();
	auto data = new dx_swapchain_data();
	auto buffer_count = static_cast<UINT>(_params.buffer_count);

	HWND hwnd = (HWND)SDL_GetPointerProperty(
		SDL_GetWindowProperties(_window.sdl_window),
		SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL
	);
	logger::log(dx12_channel, "HWND: {}, window size: {}x{}", (void*)hwnd, _window.size.x, _window.size.y);
	logger::error_if(!hwnd, dx12_channel, "HWND is NULL");

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.BufferCount = buffer_count;
	swap_chain_desc.Width = static_cast<UINT>(_window.size.x);
	swap_chain_desc.Height = static_cast<UINT>(_window.size.y);
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain1;
	HRESULT hr = dx_expect<&IDXGIFactory4::CreateSwapChainForHwnd>(
		device_data->factory.Get(),
		cq_data->cmd_queue.Get(), hwnd,
		&swap_chain_desc, nullptr, nullptr, &swap_chain1
	);
	swap_chain1.As(&data->swap_chain);
	logger::error_if(!data->swap_chain, dx12_channel, "SwapChain3 cast failed");

	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
	rtv_heap_desc.NumDescriptors = buffer_count;
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	device_data->device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&data->rtv_heap));
	data->rtv_descriptor_size = device_data->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	data->render_targets.resize(buffer_count);
	D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = data->rtv_heap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < buffer_count; i++) {
		data->swap_chain->GetBuffer(i, IID_PPV_ARGS(&data->render_targets[i]));
		device_data->device->CreateRenderTargetView(data->render_targets[i].Get(), nullptr, rtv_handle);
		rtv_handle.ptr += data->rtv_descriptor_size;
	}

	swapchain result;
	result.engine = _device.engine;
	result.data.store(data);
	result.swapchain_size = _params.buffer_count;
	result.format = MARS_FORMAT_RGBA8_UNORM;
	result.extent = _window.size;
	return result;
}

void dx_swapchain_impl::dx_swapchain_present(const swapchain& _swapchain, const device& _device) {
	auto data = _swapchain.data.expect<dx_swapchain_data>();
	dx_expect<&IDXGISwapChain3::Present>(data->swap_chain.Get(), 0, 0);
}

size_t dx_swapchain_impl::dx_swapchain_get_back_buffer_index(const swapchain& _swapchain) {
	auto data = _swapchain.data.expect<dx_swapchain_data>();
	return data->swap_chain->GetCurrentBackBufferIndex();
}

void dx_swapchain_impl::dx_swapchain_resize(swapchain& _swapchain, const device& _device, const vector2<size_t>& _size) {
	auto data = _swapchain.data.expect<dx_swapchain_data>();
	auto device_data = _device.data.expect<dx_device_data>();
	auto buffer_count = static_cast<UINT>(data->render_targets.size());

	for (auto& rt : data->render_targets)
		rt.Reset();

	HRESULT hr = dx_expect<&IDXGISwapChain3::ResizeBuffers>(
		data->swap_chain.Get(),
		buffer_count,
		static_cast<UINT>(_size.x),
		static_cast<UINT>(_size.y),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0
	);

	D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = data->rtv_heap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < buffer_count; i++) {
		data->swap_chain->GetBuffer(i, IID_PPV_ARGS(&data->render_targets[i]));
		device_data->device->CreateRenderTargetView(data->render_targets[i].Get(), nullptr, rtv_handle);
		rtv_handle.ptr += data->rtv_descriptor_size;
	}

	_swapchain.extent = _size;
}

void dx_swapchain_impl::dx_swapchain_destroy(swapchain& _swapchain, const device& _device) {
	auto data = _swapchain.data.expect<dx_swapchain_data>();

	for (auto& rt : data->render_targets)
		rt.Reset();

	delete data;
	_swapchain = {};
}
} // namespace mars::graphics::dx

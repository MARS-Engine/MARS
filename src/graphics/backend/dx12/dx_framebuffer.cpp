#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_framebuffer.hpp>
#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {

framebuffer dx_framebuffer_impl::dx_framebuffer_create(const device& _device, const framebuffer_create_params& _params) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto fb_data = new dx_framebuffer_data();
	fb_data->is_swapchain = false;
	fb_data->render_targets.reserve(_params.views.size());
	fb_data->render_target_textures.reserve(_params.views.size());
	fb_data->before_render_states.reserve(_params.views.size());
	fb_data->rtv_handles.resize(_params.views.size());

	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
	rtv_heap_desc.NumDescriptors = static_cast<UINT>(_params.views.size());
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	device_data->device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&fb_data->rtv_heap));
	D3D12_CPU_DESCRIPTOR_HANDLE base_rtv = fb_data->rtv_heap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < _params.views.size(); ++i) {
		auto tex_data = _params.views[i].data.expect<dx_texture_data>();
		fb_data->render_targets.push_back(tex_data->resource);
		fb_data->render_target_textures.push_back(tex_data);
		fb_data->before_render_states.push_back(D3D12_RESOURCE_STATE_COMMON);
		auto rtv = base_rtv;
		rtv.ptr += i * device_data->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		fb_data->rtv_handles[i] = rtv;
		device_data->device->CreateRenderTargetView(tex_data->resource.Get(), nullptr, rtv);
	}

	framebuffer result;
	result.engine = _device.engine;
	result.data.store(fb_data);
	result.extent = _params.size;
	return result;
}

std::vector<framebuffer> dx_framebuffer_impl::dx_framebuffer_create_from_swapchain(const device& _device, const swapchain& _swapchain, const render_pass& _render_pass) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto sc_data = _swapchain.data.expect<dx_swapchain_data>();
	(void)_render_pass;

	std::vector<framebuffer> framebuffers;
	framebuffers.reserve(_swapchain.swapchain_size);

	for (size_t i = 0; i < _swapchain.swapchain_size; ++i) {
		auto fb_data = new dx_framebuffer_data();
		fb_data->render_targets.push_back(sc_data->render_targets[i]);
		fb_data->render_target_textures.push_back(nullptr);
		fb_data->before_render_states.push_back(D3D12_RESOURCE_STATE_PRESENT);
		fb_data->is_swapchain = true;

		D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = sc_data->rtv_heap->GetCPUDescriptorHandleForHeapStart();
		rtv_handle.ptr += i * sc_data->rtv_descriptor_size;
		fb_data->rtv_handles.push_back(rtv_handle);

		framebuffer fb;
		fb.engine = _device.engine;
		fb.data.store(fb_data);
		fb.extent = _swapchain.extent;
		framebuffers.push_back(fb);
	}

	return framebuffers;
}

void dx_framebuffer_impl::dx_framebuffer_destroy(framebuffer& _framebuffer, const device& _device) {
	auto data = _framebuffer.data.expect<dx_framebuffer_data>();
	delete data;
	_framebuffer = {};
}
} // namespace mars::graphics::dx

#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_framebuffer.hpp>
#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {
static DXGI_FORMAT mars_depth_format_to_dxgi(mars_format_type fmt) {
	switch (fmt) {
	case MARS_FORMAT_D32_SFLOAT:
		return DXGI_FORMAT_D32_FLOAT;
	default:
		return DXGI_FORMAT_D32_FLOAT;
	}
}

framebuffer dx_framebuffer_impl::dx_framebuffer_create(const device& _device, const framebuffer_create_params& _params) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto tex_data = _params.view.data.expect<dx_texture_data>();
	auto rp_data = _params.render_pass.data.expect<dx_render_pass_data>();

	auto fb_data = new dx_framebuffer_data();
	fb_data->render_target = tex_data->resource;
	fb_data->render_target_texture = tex_data;
	fb_data->is_swapchain = false;
	fb_data->before_render_state = D3D12_RESOURCE_STATE_COMMON;

	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
	rtv_heap_desc.NumDescriptors = 1;
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	device_data->device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&fb_data->rtv_heap));
	fb_data->rtv_handle = fb_data->rtv_heap->GetCPUDescriptorHandleForHeapStart();
	device_data->device->CreateRenderTargetView(tex_data->resource.Get(), nullptr, fb_data->rtv_handle);

	if (rp_data->depth_format != MARS_FORMAT_UNDEFINED) {
		DXGI_FORMAT dxgi_depth = mars_depth_format_to_dxgi(rp_data->depth_format);
		D3D12_RESOURCE_DESC depth_desc = {};
		depth_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depth_desc.Width = (UINT64)_params.size.x;
		depth_desc.Height = (UINT)_params.size.y;
		depth_desc.DepthOrArraySize = 1;
		depth_desc.MipLevels = 1;
		depth_desc.Format = dxgi_depth;
		depth_desc.SampleDesc.Count = 1;
		depth_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clear_value = {};
		clear_value.Format = dxgi_depth;
		clear_value.DepthStencil.Depth = rp_data->depth_clear_value;

		D3D12_HEAP_PROPERTIES heap_props = {};
		heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;

		device_data->device->CreateCommittedResource(
			&heap_props, D3D12_HEAP_FLAG_NONE,
			&depth_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value,
			IID_PPV_ARGS(&fb_data->depth_target)
		);

		D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
		dsv_heap_desc.NumDescriptors = 1;
		dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		device_data->device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&fb_data->dsv_heap));
		fb_data->dsv_handle = fb_data->dsv_heap->GetCPUDescriptorHandleForHeapStart();
		device_data->device->CreateDepthStencilView(fb_data->depth_target.Get(), nullptr, fb_data->dsv_handle);
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
	auto rp_data = _render_pass.data.expect<dx_render_pass_data>();

	Microsoft::WRL::ComPtr<ID3D12Resource> depth_resource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = {};

	if (rp_data->depth_format != MARS_FORMAT_UNDEFINED) {
		DXGI_FORMAT dxgi_depth = mars_depth_format_to_dxgi(rp_data->depth_format);
		D3D12_RESOURCE_DESC depth_desc = {};
		depth_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depth_desc.Width = (UINT64)_swapchain.extent.x;
		depth_desc.Height = (UINT)_swapchain.extent.y;
		depth_desc.DepthOrArraySize = 1;
		depth_desc.MipLevels = 1;
		depth_desc.Format = dxgi_depth;
		depth_desc.SampleDesc.Count = 1;
		depth_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clear_value = {};
		clear_value.Format = dxgi_depth;
		clear_value.DepthStencil.Depth = rp_data->depth_clear_value;

		D3D12_HEAP_PROPERTIES heap_props = {};
		heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;

		device_data->device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &depth_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value, IID_PPV_ARGS(&depth_resource));

		D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
		dsv_heap_desc.NumDescriptors = 1;
		dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		device_data->device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&dsv_heap));

		dsv_handle = dsv_heap->GetCPUDescriptorHandleForHeapStart();
		device_data->device->CreateDepthStencilView(depth_resource.Get(), nullptr, dsv_handle);
	}

	std::vector<framebuffer> framebuffers;
	framebuffers.reserve(_swapchain.swapchain_size);

	for (size_t i = 0; i < _swapchain.swapchain_size; i++) {
		auto fb_data = new dx_framebuffer_data();
		fb_data->render_target = sc_data->render_targets[i];
		fb_data->is_swapchain = true;

		D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = sc_data->rtv_heap->GetCPUDescriptorHandleForHeapStart();
		rtv_handle.ptr += i * sc_data->rtv_descriptor_size;
		fb_data->rtv_handle = rtv_handle;

		fb_data->depth_target = depth_resource;
		fb_data->dsv_heap = dsv_heap;
		fb_data->dsv_handle = dsv_handle;

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

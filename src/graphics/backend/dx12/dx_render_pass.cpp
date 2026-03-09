#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_render_pass.hpp>
#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {
render_pass dx_render_pass_impl::dx_render_pass_create(const device& _device, const render_pass_create_params& _params) {
	auto data = new dx_render_pass_data();
	data->format = _params.format;
	data->depth_format = _params.depth_format;
	data->depth_clear_value = _params.depth_clear_value;

	render_pass result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

void dx_render_pass_impl::dx_render_pass_bind(const render_pass& _render_pass, const command_buffer& _command_buffer, const framebuffer& _framebuffer, const render_pass_bind_param& _params) {
	auto cb_data = dx_expect_backend_data(_command_buffer.data.get<dx_command_buffer_data>(), __func__, "command_buffer.data");
	auto fb_data = dx_expect_backend_data(_framebuffer.data.get<dx_framebuffer_data>(), __func__, "framebuffer.data");
	auto rp_data = dx_expect_backend_data(_render_pass.data.get<dx_render_pass_data>(), __func__, "render_pass.data");

	cb_data->last_bound_framebuffer = fb_data;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = fb_data->render_target.Get();
	barrier.Transition.StateBefore = fb_data->is_swapchain
					     ? D3D12_RESOURCE_STATE_PRESENT
					     : fb_data->before_render_state;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cb_data->cmd_list->ResourceBarrier(1, &barrier);

	float clear_color[] = {_params.clear_color.x, _params.clear_color.y, _params.clear_color.z, _params.clear_color.w};
	cb_data->cmd_list->ClearRenderTargetView(fb_data->rtv_handle, clear_color, 0, nullptr);

	if (fb_data->depth_target) {
		const float clear_depth = rp_data ? _params.clear_depth : 1.0f;
		cb_data->cmd_list->ClearDepthStencilView(fb_data->dsv_handle, D3D12_CLEAR_FLAG_DEPTH, clear_depth, 0, 0, nullptr);
		cb_data->cmd_list->OMSetRenderTargets(1, &fb_data->rtv_handle, FALSE, &fb_data->dsv_handle);
	} else {
		cb_data->cmd_list->OMSetRenderTargets(1, &fb_data->rtv_handle, FALSE, nullptr);
	}
}

void dx_render_pass_impl::dx_render_pass_unbind(const render_pass& _render_pass, const command_buffer& _command_buffer) {
	auto cb_data = dx_expect_backend_data(_command_buffer.data.get<dx_command_buffer_data>(), __func__, "command_buffer.data");
	auto fb_data = cb_data->last_bound_framebuffer;
	if (!fb_data)
		return;

	D3D12_RESOURCE_STATES after_state = fb_data->is_swapchain
						? D3D12_RESOURCE_STATE_PRESENT
						: (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = fb_data->render_target.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = after_state;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cb_data->cmd_list->ResourceBarrier(1, &barrier);

	if (!fb_data->is_swapchain)
		fb_data->before_render_state = after_state;

	cb_data->last_bound_framebuffer = nullptr;
}

void dx_render_pass_impl::dx_render_pass_destroy(render_pass& _render_pass, const device& _device) {
	auto data = dx_expect_backend_data(_render_pass.data.get<dx_render_pass_data>(), __func__, "render_pass.data");
	delete data;
	_render_pass = {};
}
} // namespace mars::graphics::dx

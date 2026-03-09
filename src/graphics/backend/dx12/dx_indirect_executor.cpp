#include "dx_internal.hpp"
#include "mars/graphics/backend/dx12/dx_buffer.hpp"
#include <mars/graphics/backend/dx12/dx_indirect_executor.hpp>

#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel ie_channel("dx12");

namespace {
template <typename PipelineData>
indirect_executor create_with_constant_impl(
    const device& dev,
    PipelineData* pipe_data,
    D3D12_INDIRECT_ARGUMENT_TYPE tail_arg_type,
    UINT byte_stride,
    UINT constant_count,
    mars_command_signature_type type) {
	auto dev_data = dx_expect_backend_data(dev.data.get<dx_device_data>(), __func__, "device.data");
	indirect_executor ex;
	ex.engine = dev.engine;

	pipe_data = dx_expect_backend_data(pipe_data, __func__, "pipeline.data");
	logger::error_if(!pipe_data->has_push_constants, ie_channel,
			 "create_with_constant: pipeline has no push_constants root parameter");
	if (!pipe_data->has_push_constants || !pipe_data->root_signature)
		return ex;

	auto data = new dx_indirect_executor_data();
	data->type = type;

	D3D12_INDIRECT_ARGUMENT_DESC arg_descs[2] = {};
	arg_descs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	arg_descs[0].Constant.RootParameterIndex = (UINT)pipe_data->push_constants_root_index;
	arg_descs[0].Constant.DestOffsetIn32BitValues = 0;
	arg_descs[0].Constant.Num32BitValuesToSet = constant_count;
	arg_descs[1].Type = tail_arg_type;

	D3D12_COMMAND_SIGNATURE_DESC sig_desc = {};
	sig_desc.ByteStride = byte_stride;
	sig_desc.NumArgumentDescs = 2;
	sig_desc.pArgumentDescs = arg_descs;
	sig_desc.NodeMask = 0;

	HRESULT hr = dev_data->device->CreateCommandSignature(
	    &sig_desc, pipe_data->root_signature.Get(), IID_PPV_ARGS(&data->command_signature));
	logger::error_if(FAILED(hr), ie_channel,
			 "create_with_constant: CreateCommandSignature failed (hr={:#x})", (unsigned long)hr);
	ex.data.store(data);
	return ex;
}
} // namespace

indirect_executor dx_indirect_executor_impl::create_with_constant(const device& dev, const pipeline& pipe) {
	auto pipe_data = dx_expect_backend_data(pipe.data.get<dx_pipeline_data>(), __func__, "pipeline.data");
	return create_with_constant_impl(
	    dev,
	    pipe_data,
	    D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED,
	    28u,
	    2u,
	    MARS_COMMAND_SIGNATURE_DRAW_INDEXED);
}

indirect_executor dx_indirect_executor_impl::create_with_constant_compute(const device& dev, const compute_pipeline& pipe) {
	auto pipe_data = dx_expect_backend_data(pipe.data.get<dx_compute_pipeline_data>(), __func__, "compute_pipeline.data");
	return create_with_constant_impl(
	    dev,
	    pipe_data,
	    D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH,
	    16u,
	    1u,
	    MARS_COMMAND_SIGNATURE_DISPATCH);
}

indirect_executor dx_indirect_executor_impl::create(const device& dev, mars_command_signature_type type) {
	auto dev_data = dx_expect_backend_data(dev.data.get<dx_device_data>(), __func__, "device.data");
	auto data = new dx_indirect_executor_data();
	data->type = type;

	D3D12_INDIRECT_ARGUMENT_DESC arg_desc = {};
	UINT byte_stride = 0;

	if (type == MARS_COMMAND_SIGNATURE_DISPATCH) {
		arg_desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
		byte_stride = 12;
	} else {
		arg_desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		byte_stride = 20;
	}

	D3D12_COMMAND_SIGNATURE_DESC sig_desc = {};
	sig_desc.ByteStride = byte_stride;
	sig_desc.NumArgumentDescs = 1;
	sig_desc.pArgumentDescs = &arg_desc;
	sig_desc.NodeMask = 0;

	HRESULT hr = dev_data->device->CreateCommandSignature(
	    &sig_desc, nullptr, IID_PPV_ARGS(&data->command_signature));
	logger::error_if(FAILED(hr), ie_channel,
			 "CreateCommandSignature failed (type={}, hr={:#x})", (int)type, (unsigned long)hr);

	indirect_executor ex;
	ex.engine = dev.engine;
	ex.data.store(data);
	return ex;
}

static void do_record(
    const indirect_executor& ex,
    const command_buffer& cmd,
    const buffer& arg_buf,
    uint32_t max_count,
    const buffer* count_buf) {
	auto data = dx_expect_backend_data(ex.data.get<dx_indirect_executor_data>(), __func__, "indirect_executor.data");
	auto cb_data = dx_expect_backend_data(cmd.data.get<dx_command_buffer_data>(), __func__, "command_buffer.data");
	auto arg_data = dx_expect_backend_data(arg_buf.data.get<dx_buffer_data>(), __func__, "arg_buffer.data");

	dx_buffer_impl::dx_buffer_transition(cmd, arg_buf, MARS_BUFFER_STATE_INDIRECT_ARGUMENT);

	ID3D12Resource* count_resource = nullptr;
	UINT64 count_offset = 0;
	if (count_buf) {
		auto count_data = dx_expect_backend_data(count_buf->data.get<dx_buffer_data>(), __func__, "count_buffer.data");

		dx_buffer_impl::dx_buffer_transition(cmd, *count_buf, MARS_BUFFER_STATE_INDIRECT_ARGUMENT);

		count_resource = count_data->resource.Get();
	}

	cb_data->cmd_list->ExecuteIndirect(
	    data->command_signature.Get(),
	    max_count,
	    arg_data->resource.Get(),
	    0,
	    count_resource,
	    count_offset);
}

void dx_indirect_executor_impl::record_dispatch(
    const indirect_executor& ex, const command_buffer& cmd,
    const buffer& arg_buf, uint32_t max_count, const buffer* count_buf) {
	do_record(ex, cmd, arg_buf, max_count, count_buf);
}

void dx_indirect_executor_impl::record_draw_indexed(
    const indirect_executor& ex, const command_buffer& cmd,
    const buffer& arg_buf, uint32_t max_count, const buffer* count_buf) {
	do_record(ex, cmd, arg_buf, max_count, count_buf);
}

void dx_indirect_executor_impl::destroy(indirect_executor& ex) {
	auto data = dx_expect_backend_data(ex.data.get<dx_indirect_executor_data>(), __func__, "indirect_executor.data");
	delete data;
	ex = {};
}
} // namespace mars::graphics::dx

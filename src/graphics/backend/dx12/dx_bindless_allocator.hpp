#pragma once

#include "dx_internal.hpp"

#include <mars/debug/logger.hpp>

#include <algorithm>
#include <ranges>
#include <utility>
#include <vector>

namespace mars::graphics::dx {

inline log_channel& dx_bindless_allocator_channel() {
	static log_channel channel("dx12.bindless");
	return channel;
}

inline void dx_merge_bindless_ranges(std::vector<std::pair<UINT, UINT>>& _ranges) {
	if (_ranges.empty())
		return;

	std::ranges::sort(_ranges, [](const auto& _lhs, const auto& _rhs) {
		return _lhs.first < _rhs.first;
	});

	size_t write_index = 0;
	for (size_t read_index = 1; read_index < _ranges.size(); ++read_index) {
		auto& current = _ranges[write_index];
		const auto& next = _ranges[read_index];
		const UINT current_end = current.first + current.second;
		if (next.first <= current_end) {
			const UINT next_end = next.first + next.second;
			current.second = (std::max)(current_end, next_end) - current.first;
			continue;
		}
		_ranges[++write_index] = next;
	}
	_ranges.resize(write_index + 1);
}

inline UINT dx_allocate_bindless_range(std::vector<std::pair<UINT, UINT>>& _free_ranges, UINT& _next_slot, UINT _limit_exclusive, UINT _count, const char* _label) {
	assert(_count > 0);

	for (size_t index = 0; index < _free_ranges.size(); ++index) {
		auto& range = _free_ranges[index];
		if (range.second < _count)
			continue;

		const UINT start = range.first;
		range.first += _count;
		range.second -= _count;
		if (range.second == 0)
			_free_ranges.erase(_free_ranges.begin() + static_cast<std::ptrdiff_t>(index));
		return start;
	}

	if (_next_slot > _limit_exclusive || _count > (_limit_exclusive - _next_slot)) {
		logger::error(dx_bindless_allocator_channel(), "bindless {} descriptor heap exhausted (requested={}, next={}, limit={})", _label, _count, _next_slot, _limit_exclusive);
		assert(false && "bindless descriptor heap exhausted");
		return UINT32_MAX;
	}

	const UINT start = _next_slot;
	_next_slot += _count;
	return start;
}

inline void dx_release_bindless_range(std::vector<std::pair<UINT, UINT>>& _free_ranges, UINT _start, UINT _count) {
	if (_start == UINT32_MAX || _count == 0)
		return;

	_free_ranges.push_back({_start, _count});
	dx_merge_bindless_ranges(_free_ranges);
}

inline UINT dx_allocate_bindless_srv_slot(dx_device_data* _device_data) {
	return dx_allocate_bindless_range(
		_device_data->free_bindless_srv_ranges,
		_device_data->next_bindless_srv_idx,
		dx_device_data::BINDLESS_SRV_BASE + dx_device_data::BINDLESS_SRV_COUNT,
		1,
		"srv"
	);
}

inline UINT dx_allocate_bindless_uav_range(dx_device_data* _device_data, UINT _count) {
	return dx_allocate_bindless_range(
		_device_data->free_bindless_uav_ranges,
		_device_data->next_bindless_uav_idx,
		dx_device_data::BINDLESS_UAV_BASE + dx_device_data::BINDLESS_UAV_COUNT,
		_count,
		"uav"
	);
}

inline void dx_release_bindless_srv_slot(dx_device_data* _device_data, UINT _slot) {
	dx_release_bindless_range(_device_data->free_bindless_srv_ranges, _slot, 1);
}

inline void dx_release_bindless_uav_range(dx_device_data* _device_data, UINT _start, UINT _count) {
	dx_release_bindless_range(_device_data->free_bindless_uav_ranges, _start, _count);
}

} // namespace mars::graphics::dx

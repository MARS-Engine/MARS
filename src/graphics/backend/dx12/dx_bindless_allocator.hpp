#pragma once

#include "dx_internal.hpp"

#include <mars/debug/logger.hpp>

#include <algorithm>
#include <utility>
#include <vector>

namespace mars::graphics::dx {

inline log_channel& dx_bindless_allocator_channel() {
	static log_channel channel("dx12.bindless");
	return channel;
}

inline void dx_merge_bindless_ranges(std::vector<std::pair<UINT, UINT>>& ranges) {
	if (ranges.empty())
		return;

	std::sort(ranges.begin(), ranges.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.first < rhs.first;
	});

	size_t write_index = 0;
	for (size_t read_index = 1; read_index < ranges.size(); ++read_index) {
		auto& current = ranges[write_index];
		const auto& next = ranges[read_index];
		const UINT current_end = current.first + current.second;
		if (next.first <= current_end) {
			const UINT next_end = next.first + next.second;
			current.second = (std::max)(current_end, next_end) - current.first;
			continue;
		}
		ranges[++write_index] = next;
	}
	ranges.resize(write_index + 1);
}

inline UINT dx_allocate_bindless_range(
    std::vector<std::pair<UINT, UINT>>& free_ranges,
    UINT& next_slot,
    UINT limit_exclusive,
    UINT count,
    const char* label) {
	assert(count > 0);

	for (size_t index = 0; index < free_ranges.size(); ++index) {
		auto& range = free_ranges[index];
		if (range.second < count)
			continue;

		const UINT start = range.first;
		range.first += count;
		range.second -= count;
		if (range.second == 0)
			free_ranges.erase(free_ranges.begin() + static_cast<std::ptrdiff_t>(index));
		return start;
	}

	if (next_slot > limit_exclusive || count > (limit_exclusive - next_slot)) {
		logger::error(
		    dx_bindless_allocator_channel(),
		    "bindless {} descriptor heap exhausted (requested={}, next={}, limit={})",
		    label,
		    count,
		    next_slot,
		    limit_exclusive);
		assert(false && "bindless descriptor heap exhausted");
		return UINT32_MAX;
	}

	const UINT start = next_slot;
	next_slot += count;
	return start;
}

inline void dx_release_bindless_range(
    std::vector<std::pair<UINT, UINT>>& free_ranges,
    UINT start,
    UINT count) {
	if (start == UINT32_MAX || count == 0)
		return;

	free_ranges.push_back({start, count});
	dx_merge_bindless_ranges(free_ranges);
}

inline UINT dx_allocate_bindless_srv_slot(dx_device_data* device_data) {
	return dx_allocate_bindless_range(
	    device_data->free_bindless_srv_ranges,
	    device_data->next_bindless_srv_idx,
	    dx_device_data::BINDLESS_SRV_BASE + dx_device_data::BINDLESS_SRV_COUNT,
	    1,
	    "srv");
}

inline UINT dx_allocate_bindless_uav_range(dx_device_data* device_data, UINT count) {
	return dx_allocate_bindless_range(
	    device_data->free_bindless_uav_ranges,
	    device_data->next_bindless_uav_idx,
	    dx_device_data::BINDLESS_UAV_BASE + dx_device_data::BINDLESS_UAV_COUNT,
	    count,
	    "uav");
}

inline void dx_release_bindless_srv_slot(dx_device_data* device_data, UINT slot) {
	dx_release_bindless_range(device_data->free_bindless_srv_ranges, slot, 1);
}

inline void dx_release_bindless_uav_range(dx_device_data* device_data, UINT start, UINT count) {
	dx_release_bindless_range(device_data->free_bindless_uav_ranges, start, count);
}

} // namespace mars::graphics::dx

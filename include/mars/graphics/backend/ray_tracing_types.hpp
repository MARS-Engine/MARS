#pragma once

#include <cstdint>

namespace mars {

struct rt_dispatch_region {
	uint64_t address = 0;
	uint32_t stride  = 0;
	uint32_t size    = 0;
};

struct rt_dispatch_regions {
	rt_dispatch_region raygen;
	rt_dispatch_region miss;
	rt_dispatch_region hit;
	rt_dispatch_region callable;
};

} // namespace mars

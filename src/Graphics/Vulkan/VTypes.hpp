#ifndef __VTYPES__
#define __VTYPES__

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <iostream>
#include <vector>
#include "Debug/Debug.hpp"

using namespace std;

#define VK_CHECK(x) do { VkResult err = x; if (err) { Debug::Error("Detected Vulkan error: " + err);  abort(); } } while (0)

struct VertexInputDescription {

    vector<VkVertexInputBindingDescription> bindings;
    vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;
};

#endif
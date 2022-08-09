#ifndef __VFRAMEBUFFER__
#define __VFRAMEBUFFER__

#include "VTypes.hpp"
#include <vector>

using namespace std;

class VSwapchain;
class VRenderPass;

class VFramebuffer {
public:
    vector<VkFramebuffer> rawFramebuffers;

    VSwapchain* swapchain;
    VRenderPass* renderPass;

    VFramebuffer(VSwapchain* swapchain, VRenderPass* renderPass);
    void Create();
    void Clean();
};

#endif
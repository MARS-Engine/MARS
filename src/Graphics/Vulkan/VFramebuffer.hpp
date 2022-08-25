#ifndef __VFRAME__BUFFER__
#define __VFRAME__BUFFER__

#include "VTypes.hpp"
#include "Math/Vector2.hpp"
#include <vector>

using namespace std;

class VSwapchain;
class VRenderPass;
class Texture;

class VFramebuffer {
public:
    vector<VkFramebuffer> rawFramebuffers;

    VSwapchain* swapchain;
    VRenderPass* renderPass;

    void Create(VSwapchain* swapchain, VRenderPass* renderPass);
    void Create(Vector2 size, vector<Texture*> textures);
    void Clean();
};

#endif
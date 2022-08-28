#ifndef __RENDER__PASS__MANAGER__
#define __RENDER__PASS__MANAGER__

#include <vector>
#include "Graphics/Vulkan/VRenderPass.hpp"
#include "Graphics/VEngine.hpp"

using namespace std;

class RenderPassManager {
public:
    static vector<VRenderPass*> renderPasses;

    static VRenderPass* GetRenderPass(const string& name, VEngine* engine, RenderPassData type = {});
};

#endif
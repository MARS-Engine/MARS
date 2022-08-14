#ifndef __RENDERPASSMANAGER__
#define __RENDERPASSMANAGER__

#include <vector>
#include "Graphics/Vulkan/VRenderPass.hpp"
#include "Graphics/VEngine.hpp"

using namespace std;

class RenderPassManager {
public:
    static vector<VRenderPass*> renderPasses;

    static VRenderPass* GetRenderPass(const string& name, VEngine* engine);
};

#endif
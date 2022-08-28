#ifndef __VRENDERPASS__
#define __VRENDERPASS__

#include "VTypes.hpp"
#include "Math/Vector2.hpp"
#include "../Texture.hpp"
#include <vector>

using namespace std;

class VDevice;
class VDepth;

struct RenderPassData {
    bool shouldLoad = true;
    bool swapchainReady = false;

    VkImageLayout GetInitialImageLayout();
    VkImageLayout GetFinalImageLayout();
};

class VRenderPass {
private:
    vector<VkSubpassDependency> dependencies;
    vector<VkAttachmentDescription> descriptions;
    VkSubpassDescription subpass{};
    VkRenderPassCreateInfo renderPassInfo;
public:
    //Vulkan
    vector<VkAttachmentReference> attachments;

    //MVRE
    VDevice* device = nullptr;
    VDepth* depth = nullptr;
    VmaAllocator allocator;

    //Render Pass
    string name;
    VkRenderPass rawRenderPass;
    RenderPassData type;

    VRenderPass(VmaAllocator& allocator, VDevice* device, RenderPassData type = {});

    void AddDescription(VkFormat format);
    void Prepare(vector<Texture*> textures);
    void AddDepth(Vector2 size);
    void Create();
    void Clean() const;
};

#endif
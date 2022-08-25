#ifndef __VRENDERPASS__
#define __VRENDERPASS__

#include "VTypes.hpp"
#include "Math/Vector2.hpp"
#include "../Texture.hpp"
#include <vector>

using namespace std;

class VDevice;
class VDepth;

class VRenderPass {
private:
    VkRenderPassCreateInfo renderPassInfo;
public:
    vector<VkSubpassDependency> dependencies;
    vector<VkAttachmentReference> attachments;
    vector<VkAttachmentDescription> descriptions;
    VkSubpassDescription subpass;
    VkRenderPass rawRenderPass;
    VDevice* device;
    VDepth* depth;
    VmaAllocator allocator;
    string name;

    VRenderPass(VmaAllocator& allocator, VDevice* device);

    void Prepare(Vector2 size, VkFormat format);
    void Prepare(vector<Texture*> textures, bool load = true);
    void Create();
    void Clean() const;
};

#endif
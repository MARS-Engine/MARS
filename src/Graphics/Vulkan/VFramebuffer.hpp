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
    vector<VkImageView> attachments;

    VRenderPass* renderPass;

    void AddAttachment(VkImageView imageView);
    void SetAttachments(vector<Texture*> textures);
    void ClearAttachments();
    void Create(VRenderPass* renderPass, Vector2 size);
    void Clean();
};

#endif
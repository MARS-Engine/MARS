#ifndef __VRENDERPASS__
#define __VRENDERPASS__

#include "vtypes.hpp"
#include "Math/vector2.hpp"
#include "../texture.hpp"
#include <vector>



class vdevice;
class vdepth;

struct render_pass_data {
    bool should_load = true;
    bool swapchain_ready = false;

    VkImageLayout get_initial_image_layout();
    VkImageLayout get_final_image_layout();
};

class vrender_pass {
private:
    std::vector<VkSubpassDependency> dependencies;
    std::vector<VkAttachmentDescription> descriptions;
    VkSubpassDescription subpass{};
    VkRenderPassCreateInfo render_pass_info;
public:
    //Vulkan
    std::vector<VkAttachmentReference> attachments;

    //MVRE
    vdevice* device = nullptr;
    vdepth* depth = nullptr;
    VmaAllocator allocator;

    //Render Pass
    std::string name;
    VkRenderPass raw_render_pass;
    render_pass_data type;

    vrender_pass(VmaAllocator& _allocator, vdevice* _device, render_pass_data _type = {});

    void add_description(VkFormat _format);
    void prepare(std::vector<texture*> _textures);
    void add_depth(vector2 _size);
    void create();
    void clean() const;
};

#endif
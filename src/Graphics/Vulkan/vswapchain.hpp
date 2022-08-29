#ifndef __VSWAPCHAIN__
#define __VSWAPCHAIN__

#include "vtypes.hpp"
#include <vector>
#include "../../Math/vector2.hpp"

using namespace std;

class vdevice;

class vswapchain {
public:
    vdevice* device;
    VkSwapchainKHR raw_swapchain;
    VkFormat format;
    vector<VkImage> images;
    vector<VkImageView> image_views;
    vector2 size;

    vswapchain(vdevice* _device);

    void create();
    void clean();
};

#endif
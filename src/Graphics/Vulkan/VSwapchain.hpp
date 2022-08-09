#ifndef __VSWAPCHAIN__
#define __VSWAPCHAIN__

#include "VTypes.hpp"
#include <vector>
#include "../../Math/Vector2.hpp"

using namespace std;

class VDevice;

class VSwapchain {
public:
    VDevice* device;
    VkSwapchainKHR rawSwapchain;
    VkFormat format;
    vector<VkImage> images;
    vector<VkImageView> imageViews;
    Vector2 size;

    VSwapchain(VDevice* device);

    void Create();
    void Clean();
};

#endif
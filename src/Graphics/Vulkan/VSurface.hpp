#ifndef __VSURFACE__
#define __VSURFACE__

#include "VTypes.hpp"

class VInstance;
class Window;

class VSurface {
public:
    VkSurfaceKHR rawSurface;
    Window* window;
    VInstance* instance;

    VSurface(Window* window, VInstance* instance);

    void Create();
    void Clean();
};

#endif
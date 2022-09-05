#ifndef __VSURFACE__
#define __VSURFACE__

#include "vtypes.hpp"

class vinstance;
class window;

class vsurface {
public:
    VkSurfaceKHR raw_surface;
    window* surf_window;
    vinstance* instance;

    vsurface(window* _window, vinstance* _instance);

    void create();
    void clean();
};

#endif
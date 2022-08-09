#ifndef __WINDOW__
#define __WINDOW__

#include "Vulkan/VTypes.hpp"
#include "../Math/Vector2.hpp"

class Window{
private:
    bool isOpen = true;
public:
    struct SDL_Window* rawWindow = nullptr;
    VkExtent2D windowSize;

    inline bool IsOpen() { return isOpen; }

    void Init(Vector2 size);
    void Process();
    void Clean();
};

#endif
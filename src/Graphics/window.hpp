#ifndef __WINDOW__
#define __WINDOW__

#include "Vulkan/vtypes.hpp"
#include "../Math/vector2.hpp"

class window{
private:
    bool _is_open = true;
public:
    struct SDL_Window* raw_window = nullptr;
    VkExtent2D window_size;
    vector2 size;

    inline bool is_open() { return _is_open; }

    void init(vector2 _size);
    void process();
    void clean();
};

#endif
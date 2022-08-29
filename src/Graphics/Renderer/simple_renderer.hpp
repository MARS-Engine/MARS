#ifndef __SIMPLE__RENDERER__
#define __SIMPLE__RENDERER__

#include "renderer_base.hpp"

class simple_renderer : public renderer_base{
public:
    using renderer_base::renderer_base;

    VkFramebuffer get_framebuffer(size_t _index) override;
    void load() override;
    void clear() override;
};

#endif
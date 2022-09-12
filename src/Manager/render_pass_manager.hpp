#ifndef __RENDER__PASS__MANAGER__
#define __RENDER__PASS__MANAGER__

#include <vector>
#include "Graphics/Vulkan/vrender_pass.hpp"
#include "Graphics/vengine.hpp"



class render_pass_manager {
public:
    static std::vector<vrender_pass*> renderPasses;

    static vrender_pass* get_render_pass(const std::string& name, vengine* engine, render_pass_data type = {});
};

#endif
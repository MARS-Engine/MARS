#ifndef __DEFERREDHANDLER__
#define __DEFERREDHANDLER__

#include "../CommandBuffer.hpp"
#include "Manager/PipelineManager.hpp"
#include "Manager/RenderPassManager.hpp"
#include "Manager/ShaderManager.hpp"
#include "../ShaderData.hpp"
#include "../Texture.hpp"
#include "../Vulkan/VTypes.hpp"

class VEngine;

class DeferredHandler {
public:
    CommandBuffer* commandBuffer;
    VEngine* engine;
    ShaderData* data;
    Shader* shader;
    Pipeline* pipeline;
    vector<Texture*> textures;
    VkFramebuffer framebuffer;

    DeferredHandler(VEngine* engine);
    void Load();
    void Update();
};

#endif
#include "DeferredRenderer.hpp"
#include "../Window.hpp"
#include "../Vulkan/VTexture.hpp"
#include "../Vulkan/VDevice.hpp"
#include "../Vulkan/VPipeline.hpp"
#include "Manager/LightManager.hpp"
#include "../Vulkan/VDepth.hpp"

void DeferredRenderer::CreateTexture(const string& name, DEFERRED_TEXTURE_TYPE type) {
    auto texture = new Texture(engine);

    VkFormat format;

    switch (type) {
        case COLOR:
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case POSITION:
            format = VK_FORMAT_R16G16B16A16_SFLOAT;
            break;
    }

    texture->Create(engine->window->size, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    textures.push_back({ name, type, texture });
}

VRenderPass *DeferredRenderer::GetRenderPass() {
    auto renderPass = RenderPassManager::GetRenderPass("DeferredRenderer", engine);
    renderPass->Prepare(texturesData, false);
    renderPass->Create();
    return renderPass;
}

VkFramebuffer DeferredRenderer::GetFramebuffer(size_t index) {
    return framebuffer;
}

void DeferredRenderer::Clear() {
    engine->drawQueue.push_back(clearBuffer->GetCommandBuffer());
}

void DeferredRenderer::Load() {
    auto deferredRender = RenderPassManager::GetRenderPass("Renderer", engine);
    transform(textures.begin(), textures.end(), back_inserter(texturesData), [](auto& t) { return t.texture; });
    deferredRender->Prepare(texturesData);
    deferredRender->Create();

    auto shader = ShaderManager::GetShader("Engine/Assets/Shaders/Deferred.shader", engine);

    auto pipeline = PipelineManager::GetPipeline("Renderer");

    if (pipeline == nullptr) {
        pipeline = new Pipeline(engine, shader, RenderPassManager::GetRenderPass("default", engine));
        pipeline->CreateLayout();
        pipeline->pipeline->viewport.height = -pipeline->pipeline->viewport.height;
        pipeline->pipeline->viewport.y = -pipeline->pipeline->viewport.height;
        pipeline->Create();
        PipelineManager::AddPipeline("Renderer", pipeline);
    }

    data = new ShaderData(shader, engine);
    for (auto& t : textures)
        data->GetUniform(t.name)->SetTexture(t.texture);
    LightManager::GenerateShaderUniform(data);
    data->Generate();

    renderBuffer = new CommandBuffer(engine);
    renderBuffer->renderPass = RenderPassManager::GetRenderPass("default", engine);
    renderBuffer->Create();

    for (int i = 0; i < VEngine::FRAME_OVERLAP; i++) {
        renderBuffer->Begin(i);
        renderBuffer->LoadDefault(i);
        pipeline->Bind(renderBuffer);
        data->Bind(renderBuffer, pipeline);
        renderBuffer->Draw(3, 1);
        renderBuffer->End();
    }

    vector<VkImageView> attachments;
    for (auto& t : texturesData)
        attachments.push_back(t->vTexture->imageView);
    attachments.push_back(RenderPassManager::GetRenderPass("default", engine)->depth->imageView);

    VkFramebufferCreateInfo fbufCreateInfo = {};
    fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbufCreateInfo.pNext = NULL;
    fbufCreateInfo.renderPass = deferredRender->rawRenderPass;
    fbufCreateInfo.pAttachments = attachments.data();
    fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    fbufCreateInfo.width = engine->window->windowSize.width;
    fbufCreateInfo.height = engine->window->windowSize.height;
    fbufCreateInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(engine->device->rawDevice, &fbufCreateInfo, nullptr, &framebuffer));

    clearBuffer = new CommandBuffer(engine);
    clearBuffer->renderPass = GetRenderPass();
    clearBuffer->Create();
    clearBuffer->vCommandBuffer->frame = GetFramebuffer(0);

    for (int i = 0; i < VEngine::FRAME_OVERLAP; i++) {
        clearBuffer->Begin(i);
        clearBuffer->LoadDefault(i);
        clearBuffer->End();
    }
}

void DeferredRenderer::Update() {
    LightManager::UpdateShaderUniform(data);
}

void DeferredRenderer::Render() {
    engine->drawQueue.push_back(renderBuffer->GetCommandBuffer());
}
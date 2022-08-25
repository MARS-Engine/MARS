#include "DeferredHandler.hpp"
#include "../Window.hpp"
#include "../Vulkan/VTexture.hpp"
#include "../Vulkan/VDevice.hpp"
#include "../Vulkan/VPipeline.hpp"
#include "Manager/LightManager.hpp"
#include "../Vulkan/VDepth.hpp"

DeferredHandler::DeferredHandler(VEngine* _engine) {
    engine = _engine;
}

void DeferredHandler::Load() {

    textures.resize(3);

    textures[0] = new Texture(engine);
    textures[0]->Create(Vector2(engine->window->windowSize.width, engine->window->windowSize.height), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    textures[1] = new Texture(engine);
    textures[1]->Create(Vector2(engine->window->windowSize.width, engine->window->windowSize.height), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    textures[2] = new Texture(engine);
    textures[2]->Create(Vector2(engine->window->windowSize.width, engine->window->windowSize.height), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);


    auto deferredRender = RenderPassManager::GetRenderPass("Deferred", engine);
    deferredRender->Prepare(textures);
    deferredRender->Create();

    shader = ShaderManager::GetShader("Engine/Assets/Shaders/Deferred.shader", engine);

    pipeline = PipelineManager::GetPipeline("Deferred");

    if (pipeline == nullptr) {
        pipeline = new Pipeline(engine, shader, RenderPassManager::GetRenderPass("default", engine));
        pipeline->CreateLayout();
        pipeline->pipeline->viewport.height = -pipeline->pipeline->viewport.height;
        pipeline->pipeline->viewport.y = -pipeline->pipeline->viewport.height;
        pipeline->Create();
        PipelineManager::AddPipeline("Deferred", pipeline);
    }

    data = new ShaderData(shader, engine);
    data->GetUniform("samplerPosition")->SetTexture(textures[0]);
    data->GetUniform("samplerNormal")->SetTexture(textures[1]);
    data->GetUniform("samplerAlbedo")->SetTexture(textures[2]);
    data->GetUniform("GLOBAL_LIGHT")->Generate(sizeof(Sun));
    data->Generate();

    commandBuffer = new CommandBuffer(engine);
    commandBuffer->renderPass = RenderPassManager::GetRenderPass("default", engine);
    commandBuffer->Create();

    for (int i = 0; i < VEngine::FRAME_OVERLAP; i++) {
        commandBuffer->Begin(i);
        commandBuffer->LoadDefault(i);
        pipeline->Bind(commandBuffer);
        data->Bind(commandBuffer, pipeline);
        commandBuffer->Draw(3, 1);
        commandBuffer->End();
    }

    vector<VkImageView> attachments;
    attachments.push_back(textures[0]->vTexture->imageView);
    attachments.push_back(textures[1]->vTexture->imageView);
    attachments.push_back(textures[2]->vTexture->imageView);
    attachments.push_back(RenderPassManager::GetRenderPass("default", engine)->depth->imageView);

    VkFramebufferCreateInfo fbufCreateInfo = {};
    fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbufCreateInfo.pNext = NULL;
    fbufCreateInfo.renderPass = deferredRender->rawRenderPass;
    fbufCreateInfo.pAttachments = attachments.data();
    fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    fbufCreateInfo.width = textures[0]->vTexture->size.x;
    fbufCreateInfo.height = textures[0]->vTexture->size.y;
    fbufCreateInfo.layers = 1;
    VK_CHECK(vkCreateFramebuffer(engine->device->rawDevice, &fbufCreateInfo, nullptr, &framebuffer));
}

void DeferredHandler::Update() {
    data->GetUniform("GLOBAL_LIGHT")->Update(&LightManager::sun);
}
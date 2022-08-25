#include "Texture.hpp"
#include "VEngine.hpp"
#include "Vulkan/VTexture.hpp"
#include "Pipeline.hpp"

Texture::Texture(VEngine* _engine) {
    engine = _engine;
    vTexture = new VTexture(engine->device, engine->allocator);
}

Vector2 Texture::GetSize() {
    return vTexture->size;
}

void Texture::LoadTexture(string textureLocation) {
    location = textureLocation;
    vTexture->LoadTexture(textureLocation);
}

void Texture::Create(Vector2 size, VkFormat format, VkImageUsageFlagBits usage) {
    vTexture->Create(size, format, usage);
}

void Texture::Bind(CommandBuffer* commandBuffer, Pipeline* pipeline) const {
    vTexture->Bind(commandBuffer->vCommandBuffer, pipeline->pipeline);
}
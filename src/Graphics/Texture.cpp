#include "Texture.hpp"
#include "VEngine.hpp"
#include "Vulkan/VTexture.hpp"
#include "Pipeline.hpp"

Texture::Texture(VEngine* _engine) {
    engine = _engine;
    vTexture = new VTexture(engine->device, engine->allocator);
}

void Texture::LoadTexture(string textureLocation) {
    vTexture->LoadTexture(textureLocation);
}
void Texture::Bind(CommandBuffer* commandBuffer, Pipeline* pipeline) {
    vTexture->Bind(commandBuffer->vCommandBuffer, pipeline->pipeline);
}
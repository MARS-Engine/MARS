#include "FrameTexture.hpp"

FrameTexture::FrameTexture(VEngine* _engine) {
    engine = _engine;
}

void FrameTexture::Create(size_t nFrames, Vector2 frameSize, VkFormat format, VkImageUsageFlagBits usage) {
    size = frameSize;
    textures.resize(nFrames);

    for (auto& texture : textures) {
        texture = new Texture(engine);
        texture->Create(frameSize, format, usage);
    }
}
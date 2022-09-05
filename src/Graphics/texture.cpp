#include "texture.hpp"
#include "vengine.hpp"
#include "Vulkan/vtexture.hpp"
#include "pipeline.hpp"

texture::texture(vengine* _engine) {
    engine = _engine;
    base_texture = new vtexture(engine->device, engine->allocator);
}

void texture::load_texture(const string& _location) {
    location = _location;
    base_texture->load_texture(_location);
}

void texture::create(vector2 _size, VkFormat _format, VkImageUsageFlagBits _usage) {
    base_texture->create(_size, _format, _usage);
}

void texture::bind(command_buffer* _command_buffer, pipeline* _pipeline) const {
    base_texture->bind(_command_buffer->base_command_buffer, _pipeline->base_pipeline);
}
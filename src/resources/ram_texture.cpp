#include <MARS/resources/ram_texture.hpp>
#include <stb/stb_image.h>

using namespace mars_resources;

bool ram_texture::load_resource(const std::string& _path) {
    m_path = _path;
    int width, height;
    m_data = stbi_load(resources()->find_path(_path.c_str(), mars_graphics::MARS_RESOURCE_TYPE_TEXTURE).c_str(), &width, &height, &m_channels, STBI_rgb_alpha);
    m_channels = 4;

    if (m_data == nullptr)
        mars_debug::debug::error("MARS - RAM Texture - Failed to load image " + m_path);

    m_size = mars_math::vector2(width, height);
    return true;
}

void ram_texture::clean() {
    stbi_image_free(m_data);
}
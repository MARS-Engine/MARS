#include <MARS/graphics/backend/template/texture.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace mars_graphics;
using namespace mars_math;

bool texture::load_texture(const std::string& _img_path) {
    _path = _img_path;
    int width, height;
    m_data = stbi_load(_img_path.c_str(), &width, &height, &m_channels, STBI_rgb_alpha);
    m_channels = 4;

    if (m_data == nullptr) {
        mars_debug::debug::error("MARS - Texture - Failed to load image " + _img_path);
        return false;
    }

    m_size = vector2(static_cast<size_t>(width), static_cast<size_t>(height));
    return true;
}

void texture::free_og_texture() {
    stbi_image_free(m_data);
}
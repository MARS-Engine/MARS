#include <MVRE/graphics/backend/template/texture.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace mvre_graphics;
using namespace mvre_math;

bool texture::load_texture(const std::string& _img_path) {
    int width, height;
    m_data = stbi_load(_img_path.c_str(), &width, &height, &m_channels, 0);

    if (m_data == nullptr) {
        mvre_debug::debug::error("MVRE - Texture - Failed to load image " + _img_path);
        return false;
    }

    m_size = vector2(width, height);
    return true;
}

void texture::free_og_texture() {
    stbi_image_free(m_data);
}
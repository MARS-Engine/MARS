#pragma once

#include <freetype2/freetype/freetype.h>
#include <iostream>
#include <map>
#include <mars/graphics/buffer.hpp>
#include <mars/graphics/texture.hpp>
#include <mars/math/vector2.hpp>
#include <mars/math/vector3.hpp>
#include <mars/math/vector4.hpp>
#include <mars/meta.hpp>

#include <ft2build.h>
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#include <vector>
#include FT_FREETYPE_H

namespace mars {
    struct ui_instance_data {
        [[= mars::prop::buffer_entry(2)]] mars::vector3<float> position;
        [[= mars::prop::buffer_entry(3)]] mars::vector2<float> size;
        [[= mars::prop::buffer_entry(4)]] mars::vector4<float> uv_rect;
        [[= mars::prop::buffer_entry(5)]] mars::vector3<float> color;
        [[= mars::prop::buffer_entry(6)]] mars::vector4<float> radius;
    };

    namespace graphics {

        struct character {
            mars::vector2<unsigned int> position;
            mars::vector2<unsigned int> size;
            mars::vector2<int> bearing;
            int advance;
        };

        struct font {
            texture texture;
            std::map<char, character> characters;
        };

        font load_font(const char* _path) {
            font result;

            FT_Library ft_lib;
            if (FT_Init_FreeType(&ft_lib)) {
                std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
                return {};
            }

            FT_Face face;
            if (FT_New_Face(ft_lib, _path, 0, &face)) {
                std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
                return {};
            }

            FT_Set_Pixel_Sizes(face, 0, 96);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            // load max supported texture size
            int texture_size;
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_size);

            std::vector<stbrp_rect> rects;

            size_t total_character_area = 0;

            for (char c = 0x20; c <= 0x7E; ++c) {
                FT_Load_Char(face, c, FT_LOAD_RENDER);
                rects.push_back({ (int)(c - 0x20), (int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows, 0, 0, 0 });
                total_character_area += face->glyph->bitmap.width * face->glyph->bitmap.rows;
            }

            texture_size = std::min<int>(texture_size, pow(2, ceil(log((std::sqrt(total_character_area * 1.2f))) / log(2))));

            stbrp_context ctx;
            std::vector<stbrp_node> nodes(texture_size);
            stbrp_init_target(&ctx, texture_size, texture_size, nodes.data(), texture_size);
            stbrp_pack_rects(&ctx, rects.data(), rects.size());

            std::vector<uint8_t> texture_data(texture_size * texture_size, 0);

            for (auto& rect : rects) {
                char c = rect.id + 0x20;
                if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                    std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                    continue;
                }
                FT_Bitmap& bmp = face->glyph->bitmap;

                for (int row = 0; row < bmp.rows; ++row)
                    memcpy(&texture_data[(rect.y + row) * texture_size + rect.x], &bmp.buffer[row * bmp.pitch], bmp.width);

                character new_character;
                new_character.position = { (unsigned int)rect.x, (unsigned int)rect.y };
                new_character.size = { face->glyph->bitmap.width, face->glyph->bitmap.rows };
                new_character.bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
                new_character.advance = face->glyph->advance.x / 64.0f;
                result.characters[c] = new_character;
            }

            result.texture = mars::graphics::load_texture(texture_data.data(), (size_t)texture_size, (size_t)texture_size, MARS_TEXTURE_FORMAT_RED, 0);

            FT_Done_Face(face);
            FT_Done_FreeType(ft_lib);

            return result;
        }

        void render_text(font& _font, ui_instance_data* _data, const std::string& _text) {
            size_t index = 0;
            float cursorX = 0.0f;

            size_t m_size = 0;

            for (char c : _text) {
                const auto& ch = _font.characters[c];
                if (ch.size.y > m_size)
                    m_size = ch.size.y;
            }

            for (char c : _text) {
                const auto& ch = _font.characters[c];

                float xpos = cursorX + ch.bearing.x;
                float ypos = m_size - ch.bearing.y;

                _data[index].position = { xpos, ypos, 1.0f };
                _data[index].size = { (float)ch.size.x, (float)ch.size.y };

                _data[index].uv_rect = {
                    (float)ch.position.x / (float)_font.texture.size.x,
                    (float)ch.position.y / (float)_font.texture.size.y,
                    (float)ch.size.x / (float)_font.texture.size.x,
                    (float)ch.size.y / (float)_font.texture.size.y
                };

                _data[index].radius = { 20.0f, 20.0f, 20.0f, 20.0f };

                _data[index].color = { 1.f, 1.f, 0.f };

                cursorX += ch.advance;
                ++index;
            }
        }
    } // namespace graphics
} // namespace mars
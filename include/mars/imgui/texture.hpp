#pragma

#include <mars/graphics/texture.hpp>
#include <mars/imgui/struct_editor.hpp>
#include <mars/imgui/texture_browser.hpp>

namespace mars {
    namespace imgui {
        struct texture_event {
            void on_texture_selected(const graphics::texture& _texture);
            void on_texture_clicked(const graphics::texture& _texture);
        };

        template <>
        struct struct_editor<graphics::texture> : public event<texture_event>, public struct_editor_base<graphics::texture> {
          private:
            texture_browser icon_browser;

          public:
            graphics::texture& get_texture() { return *ref; }

            struct_editor<graphics::texture>();
            struct_editor<graphics::texture>(graphics::texture& _texture);

            void render(const std::string_view& _label);
        };

        template <>
        struct struct_editor<const graphics::texture> : public event<texture_event>, public struct_editor_base<const graphics::texture> {
          private:
            texture_browser icon_browser;

          public:
            const graphics::texture& get_texture() const { return *ref; }

            struct_editor<const graphics::texture>();
            struct_editor<const graphics::texture>(const graphics::texture& _texture);

            void render(const std::string_view& _label);
        };
    } // namespace imgui
} // namespace mars
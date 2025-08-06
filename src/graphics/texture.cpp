#include "imgui.h"
#include <mars/graphics/texture.hpp>
#include <mars/imgui/texture_browser.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace mars::imgui {

    bool show_modal = false;

    inline void on_texture_selected(texture_browser& _browser, const std::string& _location, graphics::texture& _value) {
        if (_value.id != 0)
            graphics::texture_destroy(_value);
        _value = graphics::load_texture(_location, 0);

        show_modal = false;
        ImGui::CloseCurrentPopup();
    }

    void struct_editor<graphics::texture>::render(graphics::texture& _value, const std::string_view& _label) {
        static texture_browser icon_browser = texture_browser("data/", "png");

        if (show_modal) {
            ImGui::OpenPopup("important_modal");
            icon_browser.listen<&texture_browser_event::on_texture_select, &on_texture_selected>(_value);
        }

        if (ImGui::BeginPopupModal("important_modal", nullptr)) {
            icon_browser.render();
            ImGui::EndPopup();
        }

        if (_value.path == "") {
            if (ImGui::Button("Select Image")) {
                show_modal = true;
            }
        } else {
            if (ImGui::ImageButton("##icon", (void*)(intptr_t)_value.id, ImVec2(100, 100))) {
                show_modal = true;
            }
        }
    }
} // namespace mars::imgui
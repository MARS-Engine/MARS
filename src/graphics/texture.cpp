#include "imgui.h"
#include <mars/graphics/texture.hpp>
#include <mars/imgui/texture_browser.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace mars::imgui {

    void struct_editor<graphics::texture>::render(graphics::texture& _value, const std::string_view& _label) {
        static bool show_modal = false;
        static texture_browser_data icon_browser = { .base_location = "data/", .format = "png", .dirty = true };

        if (show_modal) {
            ImGui::OpenPopup("important_modal");
        }

        if (ImGui::BeginPopupModal("important_modal", nullptr)) {
            if (texture_browser(icon_browser)) {
                show_modal = false;
                _value = mars::graphics::load_texture(icon_browser.clicked, 0);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (_value.path == "") {
            if (ImGui::Button("Select Image")) {
                show_modal = true;
            }
        } else {
            if (ImGui::ImageButton("##icon", (void*)(intptr_t)_value.id, ImVec2(100, 100))) {
            }
        }
    }
} // namespace mars::imgui
#include "mars/graphics/texture.hpp"
#include "imgui.h"
#include <mars/imgui/texture.hpp>
#include <mars/imgui/texture_browser.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace mars::imgui {

    void on_texture_selected(texture_browser& _browser, const std::string& _location, struct_editor<graphics::texture>& _value) {
        if (_value.get_texture().id != 0)
            graphics::texture_destroy(_value.get_texture());
        _value.get_texture() = graphics::texture_load(_location, 0);

        ImGui::CloseCurrentPopup();
    }

    struct_editor<graphics::texture>::struct_editor() : icon_browser("data/", "png") {}
    struct_editor<graphics::texture>::struct_editor(graphics::texture& _texture) : icon_browser("data/", "png") {
        this->ref = &_texture;
        icon_browser.listen<&texture_browser_event::on_texture_select, &on_texture_selected>(*this);
    }

    struct_editor<const graphics::texture>::struct_editor() : icon_browser("data/", "png") {}
    struct_editor<const graphics::texture>::struct_editor(const graphics::texture& _texture) : icon_browser("data/", "png") {
        this->ref = &_texture;
    }

    void struct_editor<graphics::texture>::render(const std::string_view& _label) {
        if (ImGui::BeginPopupModal("important_modal", nullptr)) {
            icon_browser.render();
            ImGui::EndPopup();
        }

        if (ref->path == "") {
            if (ImGui::Button("Select Image")) {
                ImGui::OpenPopup("important_modal");
            }
        } else {
            if (ImGui::ImageButton("##icon", (void*)(intptr_t)ref->id, ImVec2(100, 100))) {
                ImGui::OpenPopup("important_modal");
            }
        }
    }

    void struct_editor<const graphics::texture>::render(const std::string_view& _label) {
        if (ref->path == "") {
            if (ImGui::Button("No Image", ImVec2(100, 100)))
                broadcast<&texture_event::on_texture_clicked>(*ref);
        } else if (ImGui::ImageButton("##icon", (void*)(intptr_t)ref->id, ImVec2(100, 100))) {
            broadcast<&texture_event::on_texture_clicked>(*ref);
        }
    }
} // namespace mars::imgui
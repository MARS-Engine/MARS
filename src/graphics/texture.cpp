#include <mars/graphics/texture.hpp>
#include <mars/imgui/browser.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace mars::imgui {

    void struct_editor<graphics::texture>::render(graphics::texture& _value, const std::string_view& _label) {
        static bool show_modal = true;
        static browser_data icon_browser = { .location = "data/", .format = "png" };
        static std::string location = "data/";

        if (show_modal) {
            ImGui::OpenPopup("important_modal");
        }

        if (ImGui::BeginPopupModal("important_modal", nullptr)) {
            ImGui::Text("SELECT A IMAGE!!!!");

            location = render_browser(location, icon_browser);

            ImGui::EndPopup();
        }

        if (ImGui::Button("Select Image")) {
            show_modal = true;
        }
    }
} // namespace mars::imgui
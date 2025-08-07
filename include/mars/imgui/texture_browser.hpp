#pragma once

#include "breadcrumb.hpp"
#include "mars/event/event.hpp"
#include <imgui.h>
#include <mars/graphics/texture.hpp>
#include <mars/io/file.hpp>
#include <vector>

namespace mars {
    namespace imgui {
        struct texture_browser_entry {
            std::string name;
            graphics::texture texture;
        };

        inline bool image_button(ImTextureID image_texture, const ImVec2& image_size, const char* label) {
            ImGuiContext& g = *ImGui::GetCurrentContext();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 text_size = ImGui::CalcTextSize(label);

            float spacing = ImGui::GetStyle().ItemInnerSpacing.y;
            ImVec2 total_size = {
                std::max(image_size.x, text_size.x),
                image_size.y + spacing + text_size.y
            };

            ImGui::InvisibleButton(label, total_size);
            bool pressed = ImGui::IsItemClicked();

            ImVec2 image_pos = {
                cursor_pos.x + (total_size.x - image_size.x) * 0.5f,
                cursor_pos.y
            };

            ImVec2 text_pos = {
                cursor_pos.x + (total_size.x - text_size.x) * 0.5f,
                cursor_pos.y + image_size.y + spacing
            };

            draw_list->AddImage(image_texture, image_pos, ImVec2(image_pos.x + image_size.x, image_pos.y + image_size.y));
            draw_list->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), label);

            if (ImGui::IsItemHovered()) {
                draw_list->AddRect(cursor_pos, ImVec2(cursor_pos.x + total_size.x, cursor_pos.y + total_size.y), ImGui::GetColorU32(ImGuiCol_ButtonHovered), 5.0f);
            }

            return pressed;
        }

        struct texture_browser_event {
            void on_texture_select(struct texture_browser& _browser, const std::string& _location);
        };

        class texture_browser : public event<texture_browser_event> {
          private:
            breadcrumb path_breadcrumb;
            std::vector<texture_browser_entry> entries;
            std::string base_location;
            std::string location;
            std::string format;
            std::string clicked;

            inline void process_location() {
                // clear cache
                for (texture_browser_entry& entry : entries)
                    graphics::texture_destroy(entry.texture);
                entries.clear();

                std::string ref_loc = location;

                if (location[0] != '.')
                    ref_loc = "./" + ref_loc;

                // get files
                std::vector<std::string> files = io::get_file_names_in_folder(ref_loc);

                // filte out by format
                if (format != "") {
                    auto ends_with = [](const std::string& str, const std::string& suffix) -> bool {
                        return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
                    };

                    auto result = files | std::views::filter([&](const std::string& name) { return ends_with(name, '.' + format); });
                    files = std::vector<std::string>(result.begin(), result.end());
                }

                // load new files
                for (std::string& file : files)
                    entries.emplace_back(file, mars::graphics::texture_load(ref_loc + file, 0));
            }

          public:
            inline static void on_breadcrumb_changed(breadcrumb& _bread, const std::string& _location, texture_browser& _browser) {
                _browser.location = _location;
                _browser.process_location();
            }

            inline texture_browser(const std::string& _location, const std::string& _format = "") : location(_location), format(_format), path_breadcrumb(_location) {
                path_breadcrumb.listen<&breadcrumb_event::on_location_changed, &texture_browser::on_breadcrumb_changed>(*this);
                process_location();
            }

            ~texture_browser() {
                path_breadcrumb.stop_listening<&breadcrumb_event::on_location_changed, &texture_browser::on_breadcrumb_changed>(*this);
            }

            void render() {
                path_breadcrumb.render();

                float panel_width = ImGui::GetContentRegionAvail().x;
                float padding = 4;
                int items_per_row = std::max(1, (int)((panel_width + padding) / (100 + padding)));

                std::vector<std::string> folders = io::get_subdirectories(location);

                int i = 0;
                for (std::string& folder : folders) {
                    std::string render_txt = "📁" + folder;
                    if (ImGui::Button(render_txt.c_str(), ImVec2(100, 100))) {
                        location += folder + "/";
                        path_breadcrumb.set_path(location);
                    }

                    if ((i + 1) % items_per_row != 0)
                        ImGui::SameLine(0.0f, padding);
                    i++;
                }

                for (texture_browser_entry& entry : entries) {
                    ImGui::PushID(i);
                    if (ImGui::ImageButton("##unique_button_id", (void*)(intptr_t)entry.texture.id, ImVec2(100, 100)))
                        broadcast<&texture_browser_event::on_texture_select>(*this, location + entry.name);

                    if ((i + 1) % items_per_row != 0)
                        ImGui::SameLine(0.0f, padding);

                    i++;
                    ImGui::PopID();
                }

                ImGui::NewLine();
            }
        };
    } // namespace imgui
} // namespace mars
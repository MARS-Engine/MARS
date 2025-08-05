#pragma once

#include <imgui.h>
#include <mars/graphics/texture.hpp>
#include <mars/io/file.hpp>
#include <vector>

namespace mars {
    namespace imgui {
        struct browser_entry {
            std::string name;
            graphics::texture texture;
        };

        struct browser_data {
            std::vector<browser_entry> entries;
            std::string location;
            std::string format;
            bool dirty;
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

        inline std::string render_browser(const std::string& location, browser_data& data) {
            if (location != data.location || data.dirty) {
                data.dirty = false;
                data.location = location;
                for (browser_entry& entry : data.entries)
                    graphics::texture_destroy(entry.texture);
                data.entries.clear();

                std::string ref_loc = "./" + location;

                std::vector<std::string> files = io::get_file_names_in_folder(ref_loc);

                if (data.format != "") {
                    auto ends_with = [](const std::string& str, const std::string& suffix) -> bool {
                        return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
                    };

                    auto result = files | std::views::filter([&](const std::string& name) { return ends_with(name, '.' + data.format); });
                    files = std::vector<std::string>(result.begin(), result.end());
                }

                for (std::string& file : files)
                    data.entries.emplace_back(file, mars::graphics::load_texture(ref_loc + file, 0));
            }

            float panel_width = ImGui::GetContentRegionAvail().x;
            float padding = 4;
            int items_per_row = std::max(1, (int)((panel_width + padding) / (100 + padding)));

            std::vector<std::string> folders = io::get_subdirectories(data.location);

            int i = 0;
            for (std::string& folder : folders) {
                std::string render_txt = "📁" + folder;
                if (ImGui::Button(render_txt.c_str(), ImVec2(100, 100))) {
                    data.location += folder + "/";
                    data.dirty = true;
                }

                if ((i + 1) % items_per_row != 0)
                    ImGui::SameLine(0.0f, padding);
                i++;
            }

            for (browser_entry& entry : data.entries) {
                ImGui::PushID(i);
                ImGui::ImageButton("##unique_button_id", (void*)(intptr_t)entry.texture.id, ImVec2(100, 100));
                if ((i + 1) % items_per_row != 0)
                    ImGui::SameLine(0.0f, padding);
                i++;
                ImGui::PopID();
            }

            ImGui::NewLine();

            return data.location;
        }
    } // namespace imgui
} // namespace mars
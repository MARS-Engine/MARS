#pragma once

#include "breadcrumb.hpp"
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

        struct texture_browser_data {
            breadcrumb_data crumb_data;
            std::vector<texture_browser_entry> entries;
            std::string base_location;
            std::string location;
            std::string format;
            std::string clicked;
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

        inline bool texture_browser(texture_browser_data& _data) {
            if (_data.location == "") {
                if (_data.base_location == "")
                    _data.base_location = "./";
                _data.location = _data.base_location;
                _data.crumb_data.location = _data.location;
                _data.dirty = true;
            }

            if (_data.dirty) {
                _data.dirty = false;

                // clear cache
                for (texture_browser_entry& entry : _data.entries)
                    graphics::texture_destroy(entry.texture);
                _data.entries.clear();

                std::string ref_loc = _data.location;

                if (_data.location[0] != '.')
                    ref_loc = "./" + ref_loc;

                // get files
                std::vector<std::string> files = io::get_file_names_in_folder(ref_loc);

                // filte out by format
                if (_data.format != "") {
                    auto ends_with = [](const std::string& str, const std::string& suffix) -> bool {
                        return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
                    };

                    auto result = files | std::views::filter([&](const std::string& name) { return ends_with(name, '.' + _data.format); });
                    files = std::vector<std::string>(result.begin(), result.end());
                }

                // load new files
                for (std::string& file : files)
                    _data.entries.emplace_back(file, mars::graphics::load_texture(ref_loc + file, 0));
            }

            if (breadcrumb(_data.crumb_data)) {
                _data.location = _data.crumb_data.location;
                _data.dirty = true;
            }

            float panel_width = ImGui::GetContentRegionAvail().x;
            float padding = 4;
            int items_per_row = std::max(1, (int)((panel_width + padding) / (100 + padding)));

            std::vector<std::string> folders = io::get_subdirectories(_data.location);

            int i = 0;
            for (std::string& folder : folders) {
                std::string render_txt = "📁" + folder;
                if (ImGui::Button(render_txt.c_str(), ImVec2(100, 100))) {
                    _data.location += folder + "/";
                    _data.crumb_data.location = _data.location;
                    _data.crumb_data.dirty = true;
                    _data.dirty = true;
                }

                if ((i + 1) % items_per_row != 0)
                    ImGui::SameLine(0.0f, padding);
                i++;
            }

            for (texture_browser_entry& entry : _data.entries) {
                bool bSeleted = false;

                ImGui::PushID(i);
                if (ImGui::ImageButton("##unique_button_id", (void*)(intptr_t)entry.texture.id, ImVec2(100, 100))) {
                    _data.clicked = _data.location + entry.name;
                    bSeleted = true;
                }

                if ((i + 1) % items_per_row != 0)
                    ImGui::SameLine(0.0f, padding);

                i++;
                ImGui::PopID();

                if (bSeleted)
                    return true;
            }

            ImGui::NewLine();

            return false;
        }
    } // namespace imgui
} // namespace mars
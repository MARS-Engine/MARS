#pragma once

#include <imgui.h>
#include <mars/event/event.hpp>
#include <mars/utility/string.hpp>
#include <numeric>

namespace mars {
    namespace imgui {
        struct breadcrumb_event {
            void on_location_selected(struct breadcrumb& _crumb, const std::string& _new_location);
            void on_location_changed(struct breadcrumb& _crumb, const std::string& _new_location);
        };

        struct breadcrumb : public event<breadcrumb_event> {
            std::vector<std::string> crumbs;
            std::string location;

            breadcrumb(const std::string& _location) : location(_location) {
                update_crumbs();
            }

            void update_crumbs() {
                crumbs.clear();

                if (location == "")
                    location = "./";
                else if (location[0] != '.') {
                    if (location[0] == '/')
                        location = '.' + location;
                    else
                        location = "./" + location;
                } else if (location == ".")
                    location = "./";

                crumbs = split_string<'/'>(location);

                if (crumbs[crumbs.size() - 1] == "")
                    crumbs.pop_back();
            }

          public:
            void set_path(const std::string& _new_location) {
                location = _new_location;
                update_crumbs();
                broadcast<&breadcrumb_event::on_location_changed>(*this, location);
            }

            void render() {
                ImGui::NewLine();
                for (int i = 0; i < crumbs.size(); i++) {
                    ImGui::SameLine();

                    if (ImGui::Button(crumbs[i].c_str())) {
                        set_path(std::accumulate(std::next(crumbs.begin()), crumbs.begin() + i + 1, crumbs[0], [](std::string a, const std::string& b) {
                                     return std::move(a) + '/' + b;
                                 }) +
                                 "/");
                        broadcast<&breadcrumb_event::on_location_selected>(*this, location);
                        return;
                    }

                    ImGui::SameLine();
                    ImGui::Text("/");
                }
            }
        };
    } // namespace imgui
} // namespace mars
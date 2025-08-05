#pragma once

#include <imgui.h>
#include <mars/utility/string.hpp>
#include <numeric>

namespace mars {
    namespace imgui {
        struct breadcrumb_data {
            std::vector<std::string> crumbs;
            std::string location;
            bool dirty = false;
        };

        inline bool breadcrumb(breadcrumb_data& _data) {
            if (_data.dirty || _data.crumbs.size() == 0) {
                _data.crumbs.clear();
                _data.dirty = false;

                if (_data.location == "")
                    _data.location = "./";
                else if (_data.location[0] != '.') {
                    if (_data.location[0] == '/')
                        _data.location = '.' + _data.location;
                    else
                        _data.location = "./" + _data.location;
                } else if (_data.location == ".")
                    _data.location = "./";

                _data.crumbs = split_string<'/'>(_data.location);

                if (_data.crumbs[_data.crumbs.size() - 1] == "")
                    _data.crumbs.pop_back();
            }

            ImGui::NewLine();
            for (int i = 0; i < _data.crumbs.size(); i++) {
                ImGui::SameLine();

                if (ImGui::Button(_data.crumbs[i].c_str())) {
                    _data.location = std::accumulate(std::next(_data.crumbs.begin()), _data.crumbs.begin() + i + 1, _data.crumbs[0], [](std::string a, const std::string& b) {
                                         return std::move(a) + '/' + b;
                                     }) +
                                     "/";
                    _data.dirty = true;
                    return true;
                }

                ImGui::SameLine();
                ImGui::Text("/");
            }

            return false;
        }
    } // namespace imgui
} // namespace mars
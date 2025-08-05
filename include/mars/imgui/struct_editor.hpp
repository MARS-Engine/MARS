#pragma once

#include <imgui.h>
#include <meta>
#include <type_traits>

namespace mars::imgui {
    template <typename T>
    struct struct_editor {
        static void render(T& _value, const std::string_view& _label) {
            if constexpr (std::is_class_v<T>) {
                static constexpr auto ctx = std::meta::access_context::current();
                template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                    using C = typename[:std::meta::type_of(mem):];
                    struct_editor<C>::render(_value.[:mem:], std::define_static_string(std::meta::identifier_of(mem)));
                }
            }
        }
    };

    template <>
    struct struct_editor<size_t> {
        static void render(size_t& _value, const std::string_view& _label) {
            ImGui::InputInt(_label.begin(), reinterpret_cast<int*>(&_value));
        }
    };

    template <>
    struct struct_editor<std::string> {
        static void render(std::string& _value, const std::string_view& _label) {
            auto input_text_callback = [](ImGuiInputTextCallbackData* data) -> int {
                if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                    std::string* str = (std::string*)data->UserData;
                    str->resize(data->BufTextLen);
                    data->Buf = (char*)str->c_str();
                }
                return 0;
            };

            bool changed = false;

            ImGui::InputText(_label.begin(), (char*)_value.c_str(),
                             _value.size() + 1,
                             ImGuiInputTextFlags_CallbackResize,
                             input_text_callback,
                             &_value);
        }
    };
}; // namespace mars::imgui
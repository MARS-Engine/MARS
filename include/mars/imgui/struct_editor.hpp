#pragma once

#include "mars/meta.hpp"
#include <imgui.h>
#include <meta>
#include <type_traits>

namespace mars {
    namespace imgui {
        struct struct_editor_annotation {
            bool skip = false;
        };

        template <typename T>
        struct struct_editor_base {
          protected:
            T* ref;

          public:
            struct_editor_base() : ref(nullptr) {}
            explicit struct_editor_base(T& _value) : ref(&_value) {}

            void set_ref(T& _value) { ref = &_value; }
        };

        template <typename T>
        struct struct_editor : public struct_editor_base<T> {
          public:
            using struct_editor_base<T>::struct_editor_base;

            void render(const std::string_view& _label) {
                std::string str(_label);
                ImGui::Text("%s", str.c_str());
                ImGui::Indent(10.0f);
                if (this->ref) {
                    if constexpr (std::is_class_v<T>) {
                        static constexpr auto ctx = std::meta::access_context::current();
                        template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                            constexpr std::optional<struct_editor_annotation> value = mars::meta::get_annotation<struct_editor_annotation>(mem);
                            if constexpr (!value.has_value() || !value.value().skip) {
                                using C = typename[:std::meta::type_of(mem):];
                                struct_editor<C>(this->ref->[:mem:]).render(std::define_static_string(std::meta::identifier_of(mem)));
                            }
                        }
                    }
                }
                ImGui::Unindent(10.0f);
            }
        };

        template <>
        struct struct_editor<size_t> : public struct_editor_base<size_t> {
            using struct_editor_base<size_t>::struct_editor_base;

            void render(const std::string_view& _label) {
                if (this->ref)
                    ImGui::InputInt(_label.begin(), reinterpret_cast<int*>(ref));
            }
        };

        template <>
        struct struct_editor<std::string> : public struct_editor_base<std::string> {
            using struct_editor_base<std::string>::struct_editor_base;

            void render(const std::string_view& _label) {
                static auto input_text_callback = [](ImGuiInputTextCallbackData* data) -> int {
                    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                        std::string* str = (std::string*)data->UserData;
                        str->resize(data->BufTextLen);
                        data->Buf = (char*)str->c_str();
                    }

                    return 0;
                };

                if (this->ref)
                    ImGui::InputText(_label.begin(), (char*)ref->c_str(), ref->size() + 1, ImGuiInputTextFlags_CallbackResize, input_text_callback, &ref);
            }
        };
    } // namespace imgui

    namespace prop {
        constexpr static imgui::struct_editor_annotation struct_editor_skip = { true };
    }
} // namespace mars
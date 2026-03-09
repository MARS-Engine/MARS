#pragma once

#include <mars/meta.hpp>

#include <imgui.h>

#include <meta>
#include <string>
#include <type_traits>
#include <utility>

namespace mars::imgui {

template <typename Enum, bool Enumerable = std::meta::is_enumerable_type(^^Enum)>
	requires std::is_enum_v<Enum>
bool enum_combo(const char* label, Enum& value) {
	if constexpr (!Enumerable) {
		return false;
	} else {
		const std::string preview_label(mars::meta::enum_to_string(value));
		bool changed = false;

		if (ImGui::BeginCombo(label, preview_label.c_str())) {
			template for (constexpr auto enumerator : std::define_static_array(std::meta::enumerators_of(^^Enum))) {
				constexpr Enum option = [:enumerator:];
				const bool selected = value == option;
				const std::string option_label(mars::meta::enum_to_string(option));

				if (ImGui::Selectable(option_label.c_str(), selected)) {
					value = option;
					changed = true;
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		return changed;
	}
}

template <typename Enum, typename LabelFn, bool Enumerable = std::meta::is_enumerable_type(^^Enum)>
	requires std::is_enum_v<Enum>
bool enum_combo(const char* label, Enum& value, LabelFn&& label_fn) {
	if constexpr (!Enumerable) {
		return false;
	} else {
		const std::string preview_label(std::forward<LabelFn>(label_fn)(value));
		bool changed = false;

		if (ImGui::BeginCombo(label, preview_label.c_str())) {
			template for (constexpr auto enumerator : std::define_static_array(std::meta::enumerators_of(^^Enum))) {
				constexpr Enum option = [:enumerator:];
				const bool selected = value == option;
				const std::string option_label(std::forward<LabelFn>(label_fn)(option));

				if (ImGui::Selectable(option_label.c_str(), selected)) {
					value = option;
					changed = true;
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		return changed;
	}
}

} // namespace mars::imgui

#pragma once

#include <mars/imgui/annotation.hpp>
#include <mars/imgui/enum_combo.hpp>

#include <imgui.h>

#include <cstdint>
#include <meta>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace mars::imgui {

template <typename T>
struct struct_editor;

namespace detail {

template <typename T>
struct imgui_scalar_traits;

template <>
struct imgui_scalar_traits<int> {
	static constexpr ImGuiDataType type = ImGuiDataType_S32;
	static constexpr const char* default_format = "%d";
};

template <>
struct imgui_scalar_traits<unsigned int> {
	static constexpr ImGuiDataType type = ImGuiDataType_U32;
	static constexpr const char* default_format = "%u";
};

template <>
struct imgui_scalar_traits<std::int64_t> {
	static constexpr ImGuiDataType type = ImGuiDataType_S64;
	static constexpr const char* default_format = "%lld";
};

template <>
struct imgui_scalar_traits<std::uint64_t> {
	static constexpr ImGuiDataType type = ImGuiDataType_U64;
	static constexpr const char* default_format = "%llu";
};

template <>
struct imgui_scalar_traits<float> {
	static constexpr ImGuiDataType type = ImGuiDataType_Float;
	static constexpr const char* default_format = "%.3f";
};

template <>
struct imgui_scalar_traits<double> {
	static constexpr ImGuiDataType type = ImGuiDataType_Double;
	static constexpr const char* default_format = "%.6f";
};

template <typename T>
constexpr bool is_supported_scalar_v = requires {
	imgui_scalar_traits<std::remove_cvref_t<T>>::type;
	imgui_scalar_traits<std::remove_cvref_t<T>>::default_format;
};

template <typename T>
bool render_scalar_input(const char* label, T& value, const char* format = nullptr) {
	static_assert(is_supported_scalar_v<T>);
	using scalar_t = std::remove_cvref_t<T>;
	return ImGui::InputScalar(
	    label,
	    imgui_scalar_traits<scalar_t>::type,
	    &value,
	    nullptr,
	    nullptr,
	    format ? format : imgui_scalar_traits<scalar_t>::default_format);
}

template <typename T>
bool render_scalar_slider(const char* label, T& value, const slider_annotation<T>& annotation) {
	static_assert(is_supported_scalar_v<T>);
	using scalar_t = std::remove_cvref_t<T>;
	scalar_t min = annotation.min;
	scalar_t max = annotation.max;
	const bool changed = ImGui::SliderScalar(
	    label,
	    imgui_scalar_traits<scalar_t>::type,
	    &value,
	    &min,
	    &max,
	    annotation.format ? annotation.format : imgui_scalar_traits<scalar_t>::default_format);
	if (annotation.callback)
		annotation.callback(value);
	return changed;
}

template <typename T>
bool render_scalar_drag(const char* label, T& value, const drag_annotation<T>& annotation) {
	static_assert(is_supported_scalar_v<T>);
	using scalar_t = std::remove_cvref_t<T>;
	scalar_t min = annotation.min;
	scalar_t max = annotation.max;
	const bool changed = ImGui::DragScalar(
	    label,
	    imgui_scalar_traits<scalar_t>::type,
	    &value,
	    annotation.speed,
	    &min,
	    &max,
	    annotation.format ? annotation.format : imgui_scalar_traits<scalar_t>::default_format);
	if (annotation.callback)
		annotation.callback(value);
	return changed;
}

template <typename T>
bool render_leaf(const char* label, T& value, bool disabled) {
	if (disabled)
		ImGui::BeginDisabled();

	bool changed = false;
	using value_t = std::remove_cvref_t<T>;
	if constexpr (std::is_same_v<value_t, bool>)
		changed = ImGui::Checkbox(label, &value);
	else if constexpr (std::is_enum_v<value_t>)
		changed = mars::imgui::enum_combo(label, value);
	else if constexpr (is_supported_scalar_v<value_t>)
		changed = render_scalar_input(label, value);
	else
		ImGui::Text("%s", label);

	if (disabled)
		ImGui::EndDisabled();
	return changed;
}

} // namespace detail

template <typename T>
struct struct_editor_base {
      protected:
	T* ref;

      public:
	struct_editor_base()
	    : ref(nullptr) {
	}

	explicit struct_editor_base(T& value)
	    : ref(&value) {
	}

	void set_ref(T& value) { ref = &value; }
};

template <typename T>
struct struct_editor : public struct_editor_base<T> {
      public:
	using struct_editor_base<T>::struct_editor_base;

      public:
	bool render_contents() {
		if (!this->ref)
			return false;

		ImGui::PushID(this->ref);
		bool changed = false;
		constexpr auto ctx = std::meta::access_context::current();
		template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
			constexpr auto skip = mars::meta::get_annotation<struct_editor_annotation>(mem);
			if constexpr (skip.has_value() && skip->skip) {
				continue;
			} else {
				using member_t = typename[:std::meta::type_of(mem):];
				constexpr auto slider_anno = mars::meta::get_annotation<slider_annotation<member_t>>(mem);
				constexpr auto drag_anno = mars::meta::get_annotation<drag_annotation<member_t>>(mem);
				constexpr auto group_anno = mars::meta::get_annotation<group_annotation>(mem);
				constexpr bool is_readonly = mars::meta::has_annotation<readonly_annotation>(mem);
				constexpr const char* member_name = [] {
					constexpr auto display = mars::meta::get_annotation<mars::meta::display_annotation>(mem);
					if constexpr (display.has_value())
						return display->display_name;
					return std::define_static_string(std::meta::display_string_of(mem));
				}();
				ImGui::PushID(member_name);
				if constexpr (std::is_class_v<member_t> && !std::is_same_v<member_t, std::string>) {
					const char* group_label = member_name;
					ImGuiTreeNodeFlags flags = 0;
					if constexpr (group_anno.has_value()) {
						group_label = group_anno->label ? group_anno->label : member_name;
						if (group_anno->default_open)
							flags |= ImGuiTreeNodeFlags_DefaultOpen;
					}

					if (is_readonly)
						ImGui::BeginDisabled();
					if (ImGui::TreeNodeEx(group_label, flags)) {
						changed |= struct_editor<member_t>(this->ref->[:mem:]).render_contents();
						ImGui::TreePop();
					}
					if (is_readonly)
						ImGui::EndDisabled();
				} else if constexpr (slider_anno.has_value() && detail::is_supported_scalar_v<member_t>) {
					if (is_readonly)
						ImGui::BeginDisabled();
					changed |= detail::render_scalar_slider(member_name, this->ref->[:mem:], slider_anno.value());
					if (is_readonly)
						ImGui::EndDisabled();
				} else if constexpr (drag_anno.has_value() && detail::is_supported_scalar_v<member_t>) {
					if (is_readonly)
						ImGui::BeginDisabled();
					changed |= detail::render_scalar_drag(member_name, this->ref->[:mem:], drag_anno.value());
					if (is_readonly)
						ImGui::EndDisabled();
				} else {
					changed |= detail::render_leaf(member_name, this->ref->[:mem:], is_readonly);
				}
				ImGui::PopID();
			}
		}
		ImGui::PopID();
		return changed;
	}

      public:
	bool render(const std::string_view& label) {
		const std::string text(label);
		if constexpr (std::is_class_v<T>) {
			ImGui::Text("%s", text.c_str());
			ImGui::Indent(10.0f);
			const bool changed = render_contents();
			ImGui::Unindent(10.0f);
			return changed;
		} else if (this->ref) {
			return detail::render_leaf(text.c_str(), *this->ref, false);
		}
		return false;
	}
};

template <auto Member, auto MemberPtr, typename Parent>
bool edit_reflected_member(Parent& object) {
	using parent_t = typename mars::meta::member_pointer_info<decltype(MemberPtr)>::parent;
	static_assert(std::is_same_v<std::remove_cvref_t<Parent>, parent_t>);
	using member_t = typename[:std::meta::type_of(Member):];
	constexpr auto slider_anno = mars::meta::get_annotation<slider_annotation<member_t>>(Member);
	constexpr auto drag_anno = mars::meta::get_annotation<drag_annotation<member_t>>(Member);
	constexpr auto group_anno = mars::meta::get_annotation<group_annotation>(Member);
	constexpr bool is_readonly = mars::meta::has_annotation<readonly_annotation>(Member);
	constexpr const char* member_name = [] {
		constexpr auto display = mars::meta::get_annotation<mars::meta::display_annotation>(Member);
		if constexpr (display.has_value())
			return display->display_name;
		return std::define_static_string(std::meta::display_string_of(Member));
	}();
	ImGui::PushID(&object);
	ImGui::PushID(member_name);
	bool changed = false;
	if constexpr (std::is_class_v<member_t> && !std::is_same_v<member_t, std::string>) {
		const char* group_label = member_name;
		ImGuiTreeNodeFlags flags = 0;
		if constexpr (group_anno.has_value()) {
			group_label = group_anno->label ? group_anno->label : member_name;
			if (group_anno->default_open)
				flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		if (is_readonly)
			ImGui::BeginDisabled();
		if (ImGui::TreeNodeEx(group_label, flags)) {
			changed = struct_editor<member_t>(object.[:Member:]).render_contents();
			ImGui::TreePop();
		}
		if (is_readonly)
			ImGui::EndDisabled();
	} else if constexpr (slider_anno.has_value() && detail::is_supported_scalar_v<member_t>) {
		if (is_readonly)
			ImGui::BeginDisabled();
		changed = detail::render_scalar_slider(member_name, object.[:Member:], slider_anno.value());
		if (is_readonly)
			ImGui::EndDisabled();
	} else if constexpr (drag_anno.has_value() && detail::is_supported_scalar_v<member_t>) {
		if (is_readonly)
			ImGui::BeginDisabled();
		changed = detail::render_scalar_drag(member_name, object.[:Member:], drag_anno.value());
		if (is_readonly)
			ImGui::EndDisabled();
	} else {
		changed = detail::render_leaf(member_name, object.[:Member:], is_readonly);
	}
	ImGui::PopID();
	ImGui::PopID();
	return changed;
}

template <>
struct struct_editor<std::string> : public struct_editor_base<std::string> {
	using struct_editor_base<std::string>::struct_editor_base;

	bool render(const std::string_view& label) {
		static auto input_text_callback = [](ImGuiInputTextCallbackData* data) -> int {
			if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
				auto* str = static_cast<std::string*>(data->UserData);
				str->resize(static_cast<size_t>(data->BufTextLen));
				data->Buf = str->data();
			}

			return 0;
		};

		if (this->ref)
			return ImGui::InputText(
			    label.data(),
			    this->ref->data(),
			    this->ref->size() + 1u,
			    ImGuiInputTextFlags_CallbackResize,
			    input_text_callback,
			    this->ref);
		return false;
	}
};

} // namespace mars::imgui

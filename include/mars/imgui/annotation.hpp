#pragma once

#include <mars/meta.hpp>

namespace mars::imgui {

struct struct_editor_annotation {
	bool skip = false;
};

struct readonly_annotation {
};

struct group_annotation {
	const char* label = nullptr;
	bool default_open = true;
};

template <typename T>
using widget_callback_t = void (*)(T&);

template <typename T>
struct slider_annotation {
	T min = {};
	T max = {};
	const char* format = nullptr;
	widget_callback_t<T> callback = nullptr;
};

template <typename T>
struct drag_annotation {
	float speed = 1.0f;
	T min = {};
	T max = {};
	const char* format = nullptr;
	widget_callback_t<T> callback = nullptr;
};

inline consteval readonly_annotation readonly() {
	return {};
}

inline consteval group_annotation group(const char* label, bool default_open = true) {
	return {
		.label = label,
		.default_open = default_open,
	};
}

template <typename T>
consteval slider_annotation<T> slider(
	T min,
	T max,
	widget_callback_t<T> callback = nullptr,
	const char* format = nullptr
) {
	return {
		.min = min,
		.max = max,
		.format = format,
		.callback = callback,
	};
}

template <typename T>
consteval drag_annotation<T> drag(
	float speed,
	T min,
	T max,
	widget_callback_t<T> callback = nullptr,
	const char* format = nullptr
) {
	return {
		.speed = speed,
		.min = min,
		.max = max,
		.format = format,
		.callback = callback,
	};
}

} // namespace mars::imgui

namespace mars::prop {
constexpr static imgui::struct_editor_annotation struct_editor_skip = {true};
}

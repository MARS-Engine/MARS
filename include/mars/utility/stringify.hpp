#pragma once

#include <format>
#include <mars/meta.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace mars {

template <typename T>
struct stringify_type {
	static std::string stringify(const T& value) {
		if constexpr (std::is_enum_v<T>)
			return std::format(
				"{} ({})",
				mars::meta::enum_to_string(value),
				static_cast<std::underlying_type_t<T>>(value)
			);
		else if constexpr (std::is_pointer_v<T>)
			return value ? std::format("{}", static_cast<const void*>(value)) : "null";
		else if constexpr (std::formattable<T, char>)
			return std::format("{}", value);
		else
			return "<unformattable>";
	}
};

template <>
struct stringify_type<std::string> {
	static std::string stringify(const std::string& value) {
		return std::format("\"{}\"", value);
	}
};

template <>
struct stringify_type<std::string_view> {
	static std::string stringify(std::string_view value) {
		return std::format("\"{}\"", value);
	}
};

template <>
struct stringify_type<std::nullptr_t> {
	static std::string stringify(std::nullptr_t) {
		return "nullptr";
	}
};

template <>
struct stringify_type<const char*> {
	static std::string stringify(const char* value) {
		return value ? std::format("\"{}\"", value) : "null";
	}
};

template <>
struct stringify_type<char*> {
	static std::string stringify(const char* value) {
		return value ? std::format("\"{}\"", value) : "null";
	}
};

template <std::size_t N>
struct stringify_type<const char[N]> {
	static std::string stringify(const char (&value)[N]) {
		return std::format("\"{}\"", value);
	}
};

template <std::size_t N>
struct stringify_type<char[N]> {
	static std::string stringify(const char (&value)[N]) {
		return std::format("\"{}\"", value);
	}
};

template <>
struct stringify_type<bool> {
	static std::string stringify(bool value) {
		return value ? "true" : "false";
	}
};

template <typename T>
std::string stringify(T&& value) {
	return stringify_type<std::remove_cvref_t<T>>::stringify(std::forward<T>(value));
}

} // namespace mars

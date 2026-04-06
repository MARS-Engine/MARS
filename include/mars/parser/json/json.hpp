#pragma once

#include <cctype>
#include <array>
#include <algorithm>
#include <format>
#include <mars/math/vector3.hpp>
#include <mars/meta.hpp>
#include <mars/parser/parser.hpp>
#include <mars/utility/hex.hpp>
#include <meta>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace mars::json {
struct json_skip_annotation {
};

using json_skip_anoation = json_skip_annotation;

static constexpr json_skip_annotation skip_annotation = {};
static constexpr json_skip_annotation skip = skip_annotation;

struct json_skip_empty_annotation {
};

static constexpr json_skip_empty_annotation skip_empty = {};

struct json_skip_default_annotation {
};

static constexpr json_skip_default_annotation skip_default = {};

enum json_value_types {
	JSON_VALUE_TYPES_INVALID,
	JSON_VALUE_TYPES_STRING,
	JSON_VALUE_TYPES_OBJECT,
	JSON_VALUE_TYPES_NUMBER,
	JSON_VALUE_TYPES_ARRAY,
	JSON_VALUE_TYPES_BOOL,
};

inline json_value_types
predict_value(unsigned char _value) {
	if (_value == '{')
		return JSON_VALUE_TYPES_OBJECT;
	if (_value == '[')
		return JSON_VALUE_TYPES_ARRAY;
	if (_value == '"')
		return JSON_VALUE_TYPES_STRING;
	if (std::isdigit(_value) || _value == '-')
		return JSON_VALUE_TYPES_NUMBER;
	if (std::tolower(_value) == 't' || std::tolower(_value) == 'f')
		return JSON_VALUE_TYPES_BOOL;
	return JSON_VALUE_TYPES_INVALID;
}

inline std::string_view::iterator skip_value(std::string_view::iterator _current, std::string_view::iterator _end) {
	if (_current == _end)
		return _end;

	switch (predict_value(static_cast<unsigned char>(*_current))) {
	case JSON_VALUE_TYPES_STRING: {
		std::string parsed;
		return parse::parse_quoted_string(_current, _end, parsed);
	}
	case JSON_VALUE_TYPES_OBJECT:
	case JSON_VALUE_TYPES_ARRAY: {
		const char open_char = *_current;
		const char close_char = open_char == '{' ? '}' : ']';
		int depth = 0;
		bool inside_string = false;

		for (auto it = _current; it != _end; ++it) {
			if (*it == '"' && (it == _current || *(it - 1) != '\\'))
				inside_string = !inside_string;
			if (inside_string)
				continue;
			if (*it == open_char)
				++depth;
			else if (*it == close_char) {
				--depth;
				if (depth == 0)
					return it + 1;
			}
		}
		return _end;
	}
	case JSON_VALUE_TYPES_NUMBER: {
		auto it = _current;
		while (it != _end && *it != ',' && *it != '}' && *it != ']' && !std::isspace(static_cast<unsigned char>(*it)))
			++it;
		return it;
	}
	case JSON_VALUE_TYPES_BOOL:
		if ((_current + 4) <= _end && std::string_view(_current, _current + 4) == "true")
			return _current + 4;
		if ((_current + 5) <= _end && std::string_view(_current, _current + 5) == "false")
			return _current + 5;
		return _end;
	default:
		if ((_current + 4) <= _end && std::string_view(_current, _current + 4) == "null")
			return _current + 4;
		return _end;
	}
}

template <typename T>
struct json_type_parser;

template <typename T>
struct json_type_parser_base {
	static constexpr bool array_support = false;
	static constexpr bool string_support = false;
	static constexpr bool bool_support = false;
	static constexpr bool number_support = false;
	static constexpr bool struct_support = false;

	template <typename C>
	inline static bool should_skip_empty_field(const C& _value) {
		if constexpr (requires(const C& current) { current.empty(); })
			return _value.empty();
		return false;
	}

	template <typename C>
	inline static bool should_skip_default_field(const C& _value) {
		// Use value-initialization as the baseline "default" value. This is
		// intentionally structural and may differ from a type's semantic default.
		if constexpr (std::is_default_constructible_v<C> && requires(const C& lhs, const C& rhs) { lhs == rhs; })
			return _value == C {};
		return false;
	}

	template <typename... Args>
	inline static std::string_view::iterator default_parse(const std::string_view& _json, T& _value, Args&&... _args) {
		std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());

		if (*start != '{')
			return start;

		std::string_view::iterator current = start + 1;

		do {
			current = parse::first_space<false>(current, _json.end());

			if (*current == '}')
				return current;

			else if (current == _json.end())
				return current;

			if (*current != '"')
				return current;

			std::string name;
			auto next_current = parse::parse_quoted_string(current, _json.end(), name);
			if (next_current == current)
				return current;
			current = next_current;

			current = parse::first_space<false>(current, _json.end());

			if (*current != ':')
				return current;

			current = parse::first_space<false>(current + 1, _json.end());

			if (current == _json.end())
				return current;

			json_value_types predicted_type = predict_value(*current);

			if (predicted_type == JSON_VALUE_TYPES_INVALID)
				return current;

			constexpr auto ctx = std::meta::access_context::current();

			bool found = false;

			if constexpr (std::is_class_v<T>) {
				template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
					if (std::define_static_string(std::meta::identifier_of(mem)) != name)
						continue;

					bool valid = false;
					using C = typename[:std::meta::type_of(mem):];
					switch (predicted_type) {
					case JSON_VALUE_TYPES_STRING:
						valid = json_type_parser<C>::string_support;
						break;
					case JSON_VALUE_TYPES_OBJECT:
						valid = json_type_parser<C>::struct_support;
						break;
					case JSON_VALUE_TYPES_NUMBER:
						valid = json_type_parser<C>::number_support;
						break;
					case JSON_VALUE_TYPES_ARRAY:
						valid = json_type_parser<C>::array_support;
						break;
					case JSON_VALUE_TYPES_BOOL:
						valid = json_type_parser<C>::bool_support;
						break;
					default:
						return current;
					}

					std::string_view str{current, _json.end()};
					if (valid) {
						found = true;
						current = json_type_parser<C>::parse(str, _value.[:mem:], std::forward<Args>(_args)...);
					}
				}
			}

			if (!found) {
				size_t object_count = 0;
				bool string_count = false;
				while (current != _json.end() && (object_count != 0 || string_count || (*current != ',' && *current != '}'))) {
					current = parse::first_special(current, _json.end());
					if (current == _json.end())
						break;
					if (*current == '{')
						object_count++;
					if (*current == '"') {
						if (string_count && *(current - 1) != '\\')
							string_count = false;
						else
							string_count = true;
					} else if (*current == '}' && object_count) {
						object_count--;
						current++;
					}

					if (string_count || object_count || (*current != '}' && *current != ','))
						current++;
				}
				if (current == _json.end())
					return current;
			} else {
				current = parse::first_space<false>(current, _json.end());
			}

			if (current == _json.end())
				return current;
			if (*current == ',')
				current++;
			else if (*current != '}')
				return current;
		} while (*current != '}');

		return current + 1;
	}

	inline static void default_stringify(T& _value, std::string& _out) {
		_out += "{ ";
		constexpr auto ctx = std::meta::access_context::current();
		if constexpr (std::is_class_v<T>) {
			template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
				constexpr bool should_skip_field = mars::meta::get_annotation<json_skip_anoation>(mem).has_value();
				constexpr bool should_skip_empty_field_annotation = mars::meta::get_annotation<json_skip_empty_annotation>(mem).has_value();
				constexpr bool should_skip_default_field_annotation = mars::meta::get_annotation<json_skip_default_annotation>(mem).has_value();
				if constexpr (!should_skip_field) {
					using C = typename[:std::meta::type_of(mem):];
					const C& value = _value.[:mem:];
					bool should_skip = false;
					if constexpr (should_skip_empty_field_annotation)
						should_skip = should_skip || should_skip_empty_field(value);
					if constexpr (should_skip_default_field_annotation)
						should_skip = should_skip || should_skip_default_field(value);
					if (should_skip)
						continue;
					_out += '"' + std::string(std::define_static_string(std::meta::identifier_of(mem))) + "\":";
					json_type_parser<C>::stringify(_value.[:mem:], _out);
					_out += ',';
				}
			}
		}
		_out[_out.size() - 1] = '}';
	}
};

template <typename T>
struct json_type_parser : public json_type_parser_base<T> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, T& _value, Args&&... _args) {
		return json_type_parser_base<T>::default_parse(_json, _value, std::forward<Args>(_args)...);
	}

	inline static void stringify(T& _value, std::string& _out) {
		json_type_parser_base<T>::default_stringify(_value, _out);
	}

	static constexpr bool struct_support = true;
};

template <typename T>
struct json_type_parser<std::vector<T>> : public json_type_parser_base<std::vector<T>> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, std::vector<T>& _value, Args&&... _args) {
		std::string_view::iterator current = parse::first_space<false>(_json.begin(), _json.end());

		if (current == _json.end() || *current != '[')
			return current;

		current++;
		_value.clear();

		while (current != _json.end() && *current != ']') {
			current = parse::first_space<false>(current, _json.end());

			if (current == _json.end() || (*current != '{' && *current != '"' && *current != '[' && !std::isalnum(static_cast<unsigned char>(*current)) && *current != '-'))
				return current;

			std::string_view str{current, _json.end()};

			auto next_current = json_type_parser<T>::parse(str, _value.emplace_back(), std::forward<Args>(_args)...);
			if (next_current == current)
				return current;
			current = next_current;

			if (current == _json.end())
				return current;

			current = parse::first_space<false>(current, _json.end());

			if (current == _json.end())
				return current;
			if (*current == ',')
				current++;
			else if (*current != ']')
				return current;
		}

		return current + 1;
	}

	inline static void stringify(std::vector<T>& _value, std::string& _out) {
		if (_value.empty()) {
			_out += "[]";
			return;
		}

		_out += "[ ";

		for (T& entry : _value) {
			json_type_parser<T>::stringify(entry, _out);
			_out += ",";
		}

		_out[_out.size() - 1] = ']';
	}

	static constexpr bool array_support = true;
};

template <typename T, size_t N>
struct json_type_parser<std::array<T, N>> : public json_type_parser_base<std::array<T, N>> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, std::array<T, N>& _value, Args&&... _args) {
		std::string_view::iterator current = parse::first_space<false>(_json.begin(), _json.end());

		if (current == _json.end() || *current != '[')
			return _json.end();

		current++;
		size_t index = 0;

		while (current != _json.end() && *current != ']') {
			current = parse::first_space<false>(current, _json.end());
			if (current == _json.end() || index >= N)
				return current;

			std::string_view value_json{current, _json.end()};
			auto next_current = json_type_parser<T>::parse(value_json, _value[index], std::forward<Args>(_args)...);
			if (next_current == current)
				return current;
			current = next_current;
			index++;

			current = parse::first_space<false>(current, _json.end());
			if (current == _json.end())
				return current;
			if (*current == ',')
				current++;
			else if (*current != ']')
				return current;
		}

		if (current == _json.end() || *current != ']' || index != N)
			return current;

		return current + 1;
	}

	inline static void stringify(std::array<T, N>& _value, std::string& _out) {
		_out += '[';
		for (size_t index = 0; index < N; ++index) {
			T& current = _value[index];
			json_type_parser<T>::stringify(current, _out);
			if (index + 1 < N)
				_out += ',';
		}
		_out += ']';
	}

	static constexpr bool array_support = true;
};

template <typename K, typename V>
struct json_map_entry {
	K key {};
	V value {};
};

template <typename K, typename V>
struct json_type_parser<std::unordered_map<K, V>> : public json_type_parser_base<std::unordered_map<K, V>> {
	using map_type = std::unordered_map<K, V>;
	using entry_type = json_map_entry<K, V>;

	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, map_type& _value, Args&&... _args) {
		std::string_view::iterator current = parse::first_space<false>(_json.begin(), _json.end());

		if constexpr (std::is_same_v<K, std::string>) {
			if (current != _json.end() && *current == '{') {
				current++;
				_value.clear();

				while (current != _json.end() && *current != '}') {
					current = parse::first_space<false>(current, _json.end());
					if (current == _json.end())
						return current;

					std::string key;
					auto key_next = parse::parse_quoted_string(current, _json.end(), key);
					if (key_next == current)
						return current;
					current = key_next;

					current = parse::first_space<false>(current, _json.end());
					if (current == _json.end() || *current != ':')
						return current;

					current = parse::first_space<false>(current + 1, _json.end());
					if (current == _json.end())
						return current;

					std::string_view value_json{current, _json.end()};
					V value {};
					auto next_current = json_type_parser<V>::parse(value_json, value, std::forward<Args>(_args)...);
					if (next_current == current)
						return current;
					current = next_current;
					
					if (!_value.emplace(std::move(key), std::move(value)).second)
						return current;

					current = parse::first_space<false>(current, _json.end());
					if (current == _json.end())
						return current;
					if (*current == ',')
						current++;
					else if (*current != '}')
						return current;
				}

				if (current == _json.end() || *current != '}')
					return current;

				return current + 1;
			}
		}

		if (current == _json.end() || *current != '[')
			return current;

		current++;
		_value.clear();

		while (current != _json.end() && *current != ']') {
			current = parse::first_space<false>(current, _json.end());
			if (current == _json.end())
				return current;

			std::string_view entry_json{current, _json.end()};
			entry_type entry {};
			auto next_current = json_type_parser<entry_type>::parse(entry_json, entry, std::forward<Args>(_args)...);
			if (next_current == current)
				return current;
			current = next_current;

			if (!_value.emplace(std::move(entry.key), std::move(entry.value)).second)
				return current;

			current = parse::first_space<false>(current, _json.end());
			if (current == _json.end())
				return current;
			if (*current == ',')
				current++;
			else if (*current != ']')
				return current;
		}

		if (current == _json.end() || *current != ']')
			return current;

		return current + 1;
	}

	inline static void stringify(map_type& _value, std::string& _out) {
		struct serialized_entry {
			std::string key_text;
			entry_type entry;
		};

		std::vector<serialized_entry> entries;
		entries.reserve(_value.size());
		for (auto& [key, value] : _value) {
			std::string key_text;
			K key_copy = key;
			json_type_parser<K>::stringify(key_copy, key_text);
			entries.push_back({
				.key_text = std::move(key_text),
				.entry = {
					.key = key,
					.value = value
				}
			});
		}

		std::ranges::sort(entries, {}, &serialized_entry::key_text);

		_out += '[';
		for (size_t index = 0; index < entries.size(); ++index) {
			entry_type entry = std::move(entries[index].entry);
			json_type_parser<entry_type>::stringify(entry, _out);
			if (index + 1 < entries.size())
				_out += ',';
		}
		_out += ']';
	}

	static constexpr bool array_support = true;
};

template <>
struct json_type_parser<unsigned int> : public json_type_parser_base<unsigned int> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, unsigned int& _value, Args&&... /*_args*/) {
		std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
		std::string_view::iterator end = parse::first_special(start, _json.end());

		if (end != _json.end() && *end != ',' && *end != '}' && *end != ']')
			return start;

		std::string str(start, end);
		if (str.empty()) return start;
		for (char& c : str)
			if (!std::isdigit(c))
				return start;
		_value = std::stoul(str);
		return end;
	}

	inline static void stringify(unsigned int& _value, std::string& _out) {
		_out += std::to_string(_value);
	}

	static constexpr bool number_support = true;
};

template <>
struct json_type_parser<size_t> : public json_type_parser_base<size_t> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, size_t& _value, Args&&... /*_args*/) {
		std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
		std::string_view::iterator end = parse::first_special(start, _json.end());

		if (end != _json.end() && *end != ',' && *end != '}' && *end != ']')
			return start;

		std::string str(start, end);
		if (str.empty()) return start;
		for (char& c : str)
			if (!std::isdigit(c))
				return start;
		_value = std::stoull(str);
		return end;
	}

	inline static void stringify(size_t& _value, std::string& _out) {
		_out += std::to_string(_value);
	}

	static constexpr bool number_support = true;
};

template <>
struct json_type_parser<float> : public json_type_parser_base<float> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, float& _value, Args&&... /*_args*/) {
		std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
		std::string_view::iterator end = parse::first_special(start, _json.end());

		while (end != _json.end() && (*end == '.' || *end == '-' || *end == '+'))
			end = parse::first_special(end + 1, _json.end());

		if (end != _json.end() && *end != ',' && *end != '}' && *end != ']')
			return start;

		std::string str(start, end);
		if (str.empty()) return start;
		for (char& c : str)
			if (!std::isdigit(c) && c != '.' && c != '-' && c != '+' && c != 'e' && c != 'E')
				return start;
		_value = std::stof(str);
		return end;
	}

	inline static void stringify(float& _value, std::string& _out) {
		_out += std::to_string(_value);
	}

	static constexpr bool number_support = true;
};

template <>
struct json_type_parser<std::string> : public json_type_parser_base<std::string> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, std::string& _value, Args&&... /*_args*/) {
		std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
		std::string str;
		std::string_view::iterator end = parse::parse_quoted_string(start, _json.end(), str);
		if (end == start)
			return start;
		_value = std::move(str);
		return end;
	}

	inline static void stringify(std::string& _value, std::string& _out) {
		_out += '"';
		for (char c : _value) {
			switch (c) {
			case '"':
				_out += "\\\"";
				break;
			case '\\':
				_out += "\\\\";
				break;
			case '\n':
				_out += "\\n";
				break;
			case '\r':
				_out += "\\r";
				break;
			case '\t':
				_out += "\\t";
				break;
			case '\b':
				_out += "\\b";
				break;
			case '\f':
				_out += "\\f";
				break;
			default:
				_out += c;
				break;
			}
		}
		_out += '"';
	}

	static constexpr bool string_support = true;
};

template <>
struct json_type_parser<vector3<unsigned char>> : public json_type_parser_base<vector3<unsigned char>> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, vector3<unsigned char>& _value, Args&&... /*_args*/) {
		std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
		std::string str;
		std::string_view::iterator end = parse::parse_quoted_string(start, _json.end(), str);
		if (end == start)
			return start;
		if (str.size() != 8 || !str.starts_with("0x"))
			return start;

		_value.x = utils::hex_byte_to_char({str.begin() + 2, str.begin() + 4});
		_value.y = utils::hex_byte_to_char({str.begin() + 4, str.begin() + 6});
		_value.z = utils::hex_byte_to_char({str.begin() + 6, str.begin() + 8});

		return end;
	}

	inline static void stringify(vector3<unsigned char>& _value, std::string& _out) {
		_out += '"' + std::format("0x{:02X}{:02X}{:02X}", _value.x, _value.y, _value.z) + '"';
	}

	static constexpr bool string_support = true;
};

template <>
struct json_type_parser<bool> : public json_type_parser_base<bool> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, bool& _value, Args&&... /*_args*/) {
		std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());

		if (start == _json.end())
			return start;

		if ((start + sizeof("true") - 1) <= _json.end() && std::string_view(start, start + sizeof("true") - 1) == "true") {
			_value = true;
			return start + sizeof("true") - 1;
		} else if ((start + sizeof("false") - 1) <= _json.end() && std::string_view(start, start + sizeof("false") - 1) == "false") {
			_value = false;
			return start + sizeof("false") - 1;
		}

		return start;
	}

	inline static void stringify(bool& _value, std::string& _out) {
		_out += _value ? "true" : "false";
	}

	static constexpr bool bool_support = true;
};

template <typename T>
	requires std::is_enum_v<T>
struct json_type_parser<T> : public json_type_parser_base<T> {
	template <typename... Args>
	inline static std::string_view::iterator parse(const std::string_view& _json, T& _value, Args&&... /*_args*/) {
		std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
		std::string str;
		std::string_view::iterator end = parse::parse_quoted_string(start, _json.end(), str);
		if (end == start)
			return start;
		const std::optional<T> parsed = meta::try_string_to_enum<T>(str);
		if (!parsed.has_value())
			return start;
		_value = *parsed;
		return end;
	}

	inline static void stringify(T& _value, std::string& _out) {
		_out += '"' + std::string(meta::enum_to_string(_value)) + '"';
	}

	static constexpr bool string_support = true;
};
} // namespace mars::json

#pragma once

#include <array>
#include <charconv>
#include <cstdint>
#include <meta>
#include <string>
#include <string_view>

namespace mars::hash {
namespace detail {

static constexpr uint64_t fnv1a_seed = 0xcbf29ce484222325ull;

constexpr uint64_t fnv1a64(std::string_view _s) {
	uint64_t h = 14695981039346656037ull;
	for (unsigned char c : _s) {
		h ^= c;
		h *= 1099511628211ull;
	}
	return h;
}

constexpr void hash_append(uint64_t& _h, std::string_view _s) {
	_h ^= fnv1a64(_s) + 0x9e3779b97f4a7c15ull + (_h << 6) + (_h >> 2);
}

template<typename TLeaf>
consteval void traverse_entity(std::meta::info _r, TLeaf&& _leaf) {
	using namespace std::meta;

	if (has_identifier(_r))
		_leaf(identifier_of(_r));
	else
		_leaf(display_string_of(_r));

	if (is_namespace_member(_r)) {
		auto p = parent_of(_r);
		if (p != info{})
			traverse_entity(p, _leaf);
	}
	else {
		auto loc = source_location_of(_r);
		std::array<char, 12> buf{};

		_leaf(loc.file_name());
		_leaf(std::string_view(loc.function_name()));

		auto [p1, e1] = std::to_chars(buf.data(), buf.data() + buf.size(), loc.line());
		_leaf(std::string_view(buf.data(), p1 - buf.data()));

		auto [p2, e2] = std::to_chars(buf.data(), buf.data() + buf.size(), loc.column());
		_leaf(std::string_view(buf.data(), p2 - buf.data()));
	}
}

template<class T>
consteval uint64_t type_fingerprint_ce() {
	uint64_t h = fnv1a_seed;
	traverse_entity(std::meta::dealias(^^T), [&](std::string_view _part) {
		hash_append(h, _part);
	});
	return h;
}

inline bool encoded_part_read(std::string_view _value, std::size_t& _cursor, std::string_view& _part) {
	if (_cursor >= _value.size())
		return false;

	std::size_t len_end = _cursor;
	while (len_end < _value.size() && _value[len_end] != ':')
		++len_end;
	if (len_end == _value.size() || len_end == _cursor)
		return false;

	std::size_t len = 0;
	const auto parse_result = std::from_chars(_value.data() + _cursor, _value.data() + len_end, len);
	if (parse_result.ec != std::errc{} || parse_result.ptr != _value.data() + len_end)
		return false;

	const std::size_t part_begin = len_end + 1;
	const std::size_t part_end = part_begin + len;
	if (part_end >= _value.size() || _value[part_end] != ';')
		return false;

	_part = _value.substr(part_begin, len);
	_cursor = part_end + 1;
	return true;
}

} // namespace detail

template<class T>
constexpr uint64_t type_fingerprint_v = detail::type_fingerprint_ce<T>();

template<class T>
constexpr std::string_view type_fingerprint_string() {
	return std::define_static_string([] {
		std::string out;
		detail::traverse_entity(std::meta::dealias(^^T), [&](std::string_view _part) {
			std::array<char, 12> buf{};
			auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), _part.size());
			out += std::string_view(buf.data(), ptr - buf.data());
			out += ':';
			out += _part;
			out += ';';
		});
		return out;
	}());
}

inline uint64_t type_fingerprint_from_string(std::string_view _value) {
	uint64_t hash = detail::fnv1a_seed;
	std::size_t cursor = 0;
	std::string_view part = {};

	while (detail::encoded_part_read(_value, cursor, part))
		detail::hash_append(hash, part);

	return cursor == _value.size() ? hash : 0;
}

} // namespace mars::hash

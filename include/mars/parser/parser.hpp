#pragma once

#include <cctype>
#include <ranges>
#include <string>
#include <string_view>

namespace mars {
namespace parse {
template <bool Negate = false>
inline auto first_special(const std::string_view::iterator& begin, const std::string_view::iterator& end) {
	return std::ranges::find_if(begin, end, [](char c) -> bool {
		return (!std::isalnum(static_cast<unsigned char>(c)) && !std::isspace(static_cast<unsigned char>(c))) ^ Negate;
	});
}

template <bool Negate = false>
inline auto first_space(const std::string_view::iterator& begin, const std::string_view::iterator& end) {
	return std::ranges::find_if(begin, end, [](char c) -> bool {
		bool result = std::isspace(static_cast<unsigned char>(c));
		return result ^ !Negate;
	});
}

inline std::string_view::iterator parse_quoted_string(const std::string_view::iterator& begin, const std::string_view::iterator& end, std::string& out) {
	if (begin == end || *begin != '"')
		return end;

	out.clear();
	bool escaping = false;
	for (auto it = begin + 1; it != end; ++it) {
		const char c = *it;
		if (escaping) {
			switch (c) {
			case '"':
				out.push_back('"');
				break;
			case '\\':
				out.push_back('\\');
				break;
			case '/':
				out.push_back('/');
				break;
			case 'b':
				out.push_back('\b');
				break;
			case 'f':
				out.push_back('\f');
				break;
			case 'n':
				out.push_back('\n');
				break;
			case 'r':
				out.push_back('\r');
				break;
			case 't':
				out.push_back('\t');
				break;
			default:
				// JSON unicode escapes (\uXXXX) are not decoded yet; keep the
				// escaped payload verbatim after the backslash for now.
				out.push_back(c);
				break;
			}
			escaping = false;
			continue;
		}

		if (c == '\\') {
			escaping = true;
			continue;
		}

		if (c == '"')
			return it + 1;

		out.push_back(c);
	}

	out.clear();
	return end;
}
} // namespace parse
} // namespace mars

#pragma once

#include <mars/meta.hpp>

#include <istream>
#include <ostream>
#include <string>
#include <type_traits>

namespace mars {

template <typename T>
	requires std::is_enum_v<T>
std::ostream& operator<<(std::ostream& os, const T& value) {
	os << mars::meta::enum_to_string(value);
	return os;
}

template <typename E>
	requires std::is_enum_v<E>
std::istream& operator>>(std::istream& is, E& value) {
	std::string string_value;
	if (is >> string_value) {
		template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E))) {
			if (string_value == mars::meta::display_name(e)) {
				value = [:e:];
				return is;
			}
		}
		is.setstate(std::ios::failbit);
	}
	return is;
}

} // namespace mars

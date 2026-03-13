#pragma once

#include <mars/meta.hpp>
#include <mars/utility/enum.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <span>
#include <sstream>
#include <string_view>

namespace mars::console {

template <typename T>
auto parse_arguments(std::span<const std::string_view> args) -> T {
	T opts;

	constexpr auto ctx = std::meta::access_context::current();
	template for (constexpr auto dm : define_static_array(nonstatic_data_members_of(^^T, ctx))) {
		auto it = std::find_if(args.begin(), args.end(), [&](std::string_view arg) {
			return arg.starts_with("--") && arg.substr(2) == mars::meta::display_name(dm);
		});

		if (it == args.end()) {
			continue;
		}
		if (it + 1 == args.end()) {
			std::cerr << "argument " << *it << " is missing a value\n";
			std::exit(EXIT_FAILURE);
		}

		using value_type = typename[:type_of(dm):];
		auto iss = std::stringstream(std::string(it[1]));
		if (iss >> opts.[:dm:]; !iss) {
			std::cerr << "Failed to parse argument " << *it << " into a " << std::meta::display_string_of(std::meta::decay(^^value_type)) << '\n';
			std::exit(EXIT_FAILURE);
		}
	}

	return opts;
}

} // namespace mars::console

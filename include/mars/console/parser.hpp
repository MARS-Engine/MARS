#pragma once

#include <mars/meta.hpp>
#include <mars/utility/enum.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

namespace mars::console {

template <typename T>
auto parse_arguments(std::span<const std::string_view> args) -> T {
	T opts;

	constexpr auto ctx = std::meta::access_context::current();
	template for (constexpr auto dm : define_static_array(nonstatic_data_members_of(^^T, ctx))) {
		constexpr std::string_view option_name = mars::meta::display_name(dm);
		const std::string full_name = std::string("--") + std::string(option_name);
		auto it = std::find_if(args.begin(), args.end(), [&](std::string_view arg) {
			return arg == full_name || (arg.starts_with(full_name) && arg.size() > full_name.size() && arg[full_name.size()] == '=');
		});

		if (it == args.end())
			continue;

		std::string_view value_text;
		if (it->size() > full_name.size()) {
			value_text = it->substr(full_name.size() + 1);
			if (value_text.empty()) {
				std::cerr << "argument " << *it << " is missing a value\n";
				std::exit(EXIT_FAILURE);
			}
		} else {
			if (it + 1 == args.end()) {
				std::cerr << "argument " << *it << " is missing a value\n";
				std::exit(EXIT_FAILURE);
			}
			value_text = it[1];
		}

		using value_type = typename[:type_of(dm):];
		auto iss = std::stringstream(std::string(value_text));
		if (iss >> opts.[:dm:]; !iss) {
			std::cerr << "Failed to parse argument " << *it << " into a " << std::meta::display_string_of(std::meta::decay(^^value_type)) << '\n';
			std::exit(EXIT_FAILURE);
		}
	}

	return opts;
}

} // namespace mars::console

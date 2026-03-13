#pragma once

#include <format>
#include <functional>
#include <mars/meta.hpp>
#include <mars/utility/stringify.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#ifndef IS_SHIPPING
#define IS_SHIPPING false
#endif
#ifndef IS_DEBUG
#define IS_DEBUG true
#endif
#ifndef SAFETY_LEVEL
#define SAFETY_LEVEL 0
#endif

namespace mars {
class log_channel {
  private:
	std::string m_channel_name;

  public:
	[[nodiscard]]
	inline const std::string& name() const {
		return m_channel_name;
	}

	log_channel(const std::string& _name) {
		m_channel_name = _name;
	}

	log_channel(const char* _name) {
		m_channel_name = _name;
	}
};

namespace environment {
/**
 * Level of safety when building
 */
enum BUILD_SAFETY_LEVEL {
	BUILD_SAFETY_LEVEL_NONE = 0,
	BUILD_SAFETY_LEVEL_LOW = 1,
	BUILD_SAFETY_LEVEL_MEDIUM = 2,
	BUILD_SAFETY_LEVEL_HIGH = 3
};

constexpr bool is_shipping = IS_SHIPPING;
constexpr bool is_debug = IS_DEBUG;
constexpr unsigned int safety_level = SAFETY_LEVEL;

static_assert(is_shipping != is_debug, "IS_SHIPPING or IS_DEBUG incorrectly set");
} // namespace environment

namespace logger {

void write_formatted_line(const std::string& line);

template <typename... Args>
void mars_print(const std::string& _level, const log_channel& _channel, const std::string& _message, Args... args) {
	const std::string line = std::format("[{}] | {} | {}", _level, _channel.name(), std::vformat(_message, std::make_format_args(args...)));
	write_formatted_line(line);
}

template <typename... Args>
void log(const log_channel& _channel, const std::string& _message, Args... args) {
	mars_print("log", _channel, _message, std::forward<Args>(args)...);
}

template <typename... Args>
void warning(const log_channel& _channel, const std::string& _message, Args... args) {
	mars_print("warning", _channel, _message, std::forward<Args>(args)...);
}

template <typename... Args>
void error(const log_channel& _channel, const std::string& _message, Args... args) {
	mars_print("error", _channel, _message, std::forward<Args>(args)...);
}

template <typename... Args>
void error_if(bool _condition, const log_channel& _channel, const std::string& _message, Args... args) {
	if (_condition)
		error(_channel, _message, std::forward<Args>(args)...);
}

template <typename... Args>
[[noreturn]]
void assert_(const log_channel& _channel, const std::string& _message, Args... args) {
	std::string formatted_msg = std::vformat(_message, std::make_format_args(args...));
	mars_print("assert", _channel, "{}", formatted_msg);
	throw std::runtime_error(formatted_msg);
}

template <typename... Args>
void assert_(const bool _condition, const log_channel& _channel, const std::string& _message, Args... args) {
	if (!_condition)
		assert_(_channel, _message, std::forward<Args>(args)...);
}

template <typename Function, typename Expected, typename... Args>
decltype(auto) log_expect(const log_channel& _channel, std::string_view _function_name, Function&& _function, Expected&& _expected, Args&&... args) {
	using callable_type = std::remove_reference_t<Function>;
	using result_type = std::invoke_result_t<callable_type&, Args...>;
	static_assert(!std::is_reference_v<result_type>, "logger::log_expect does not support reference-returning callables");

	if constexpr (std::is_void_v<result_type>) {
		std::invoke(_function, std::forward<Args>(args)...);
	} else {
		result_type result = std::invoke(_function, std::forward<Args>(args)...);
		const bool ok = std::invoke(std::forward<Expected>(_expected), result);
		if (!ok) {
			std::string call_info = std::format("{}(", _function_name);
			size_t argument_index = 0u;

			template for (constexpr std::meta::info r : {^^args...}) {
				auto&& current_value = [:r:];
				if (argument_index++ != 0u)
					call_info += ", ";
				call_info += std::format("{}={}", std::meta::display_string_of(std::meta::type_of(r)), mars::stringify(current_value));
			}
			call_info += ")";

			const std::string result_info = std::format("{}={}", std::meta::display_string_of(^^result_type), mars::stringify(result));
			error(_channel, "{} -> {}", call_info, result_info);
		}
		return result;
	}
}

template <typename Function, typename Expected, typename... Args>
decltype(auto) log_expect(const log_channel& _channel, Function&& _function, Expected&& _expected, Args&&... args) {
	return log_expect(_channel, mars::meta::type_name<std::remove_cvref_t<Function>>(), std::forward<Function>(_function), std::forward<Expected>(_expected), std::forward<Args>(args)...);
}

template <auto Function, typename Expected, typename... Args>
decltype(auto) log_expect(const log_channel& _channel, Expected&& _expected, Args&&... args) {
	return log_expect(_channel, mars::meta::value_name<Function>(), Function, std::forward<Expected>(_expected), std::forward<Args>(args)...
	);
}

template <auto FunctionMember, typename Object, typename Expected, typename... Args>
	requires(std::is_member_object_pointer_v<decltype(FunctionMember)>)
decltype(auto) log_expect(const log_channel& _channel, Object&& _object, Expected&& _expected, Args&&... args) {
	return log_expect(
		_channel,
		mars::meta::value_name<FunctionMember>(),
		std::invoke(FunctionMember, std::forward<Object>(_object)),
		std::forward<Expected>(_expected),
		std::forward<Args>(args)...
	);
}

template <auto FunctionMember, typename Object, typename Expected, typename... Args>
	requires(std::is_member_function_pointer_v<decltype(FunctionMember)>)
decltype(auto) log_expect(const log_channel& _channel, Object&& _object, Expected&& _expected, Args&&... args) {
	auto invoke_member = [&_object]<typename... ForwardedArgs>(ForwardedArgs&&... forwarded_args) -> decltype(auto) {
		return std::invoke(FunctionMember, std::forward<Object>(_object), std::forward<ForwardedArgs>(forwarded_args)...);
	};
	return log_expect(
		_channel,
		mars::meta::value_name<FunctionMember>(),
		invoke_member,
		std::forward<Expected>(_expected),
		std::forward<Args>(args)...
	);
}
} // namespace logger
} // namespace mars

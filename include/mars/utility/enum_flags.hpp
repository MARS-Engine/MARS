#pragma once

#include <type_traits>

namespace mars::enum_flags {

template <typename Enum>
struct enabled : std::false_type {};

template <typename Enum>
inline constexpr bool enabled_v = enabled<Enum>::value;

} // namespace mars::enum_flags

template <typename Enum>
	requires(std::is_enum_v<Enum> && mars::enum_flags::enabled_v<Enum>)
constexpr Enum operator|(Enum _lhs, Enum _rhs) {
	using underlying_t = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<underlying_t>(_lhs) | static_cast<underlying_t>(_rhs));
}

template <typename Enum>
	requires(std::is_enum_v<Enum> && mars::enum_flags::enabled_v<Enum>)
constexpr Enum operator&(Enum _lhs, Enum _rhs) {
	using underlying_t = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<underlying_t>(_lhs) & static_cast<underlying_t>(_rhs));
}

template <typename Enum>
	requires(std::is_enum_v<Enum> && mars::enum_flags::enabled_v<Enum>)
constexpr Enum& operator|=(Enum& _lhs, Enum _rhs) {
	_lhs = _lhs | _rhs;
	return _lhs;
}

template <typename Enum>
	requires(std::is_enum_v<Enum> && mars::enum_flags::enabled_v<Enum>)
constexpr Enum& operator&=(Enum& _lhs, Enum _rhs) {
	_lhs = _lhs & _rhs;
	return _lhs;
}

template <typename Enum>
	requires(std::is_enum_v<Enum> && mars::enum_flags::enabled_v<Enum>)
constexpr Enum operator^(Enum _lhs, Enum _rhs) {
	using underlying_t = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<underlying_t>(_lhs) ^ static_cast<underlying_t>(_rhs));
}

template <typename Enum>
	requires(std::is_enum_v<Enum> && mars::enum_flags::enabled_v<Enum>)
constexpr Enum& operator^=(Enum& _lhs, Enum _rhs) {
	_lhs = _lhs ^ _rhs;
	return _lhs;
}

template <typename Enum>
	requires(std::is_enum_v<Enum> && mars::enum_flags::enabled_v<Enum>)
constexpr Enum operator~(Enum _value) {
	using underlying_t = std::underlying_type_t<Enum>;
	return static_cast<Enum>(~static_cast<underlying_t>(_value));
}

namespace mars {

template <typename Enum>
	requires(std::is_enum_v<Enum> && enum_flags::enabled_v<Enum>)
constexpr bool enum_has_flag(Enum _value, Enum _flag) {
	return (_value & _flag) == _flag;
}

namespace enum_flags_test {
enum class sample : unsigned int {
	none = 0u,
	a = 1u << 0,
	b = 1u << 1,
};
} // namespace enum_flags_test

} // namespace mars

template <>
struct mars::enum_flags::enabled<mars::enum_flags_test::sample> : std::true_type {};

static_assert((mars::enum_flags_test::sample::a | mars::enum_flags_test::sample::b) == static_cast<mars::enum_flags_test::sample>((1u << 0) | (1u << 1)));
static_assert((mars::enum_flags_test::sample::a & mars::enum_flags_test::sample::b) == mars::enum_flags_test::sample::none);
static_assert((mars::enum_flags_test::sample::a ^ mars::enum_flags_test::sample::b) == static_cast<mars::enum_flags_test::sample>((1u << 0) | (1u << 1)));
static_assert(mars::enum_has_flag(mars::enum_flags_test::sample::a | mars::enum_flags_test::sample::b, mars::enum_flags_test::sample::a));
static_assert((~mars::enum_flags_test::sample::none & mars::enum_flags_test::sample::a) == mars::enum_flags_test::sample::a);

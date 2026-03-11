#pragma once

#include "vector2.hpp"
#include "vector4.hpp"
#include <bit>
#include <cmath>
#include <cstddef>
#include <functional>
#include <numbers>
#include <type_traits>

namespace mars {
template <typename T>
struct vector3 {
	T x, y, z;

	vector3() = default;
	vector3(const vector2<T>& _xy, T _z) : x(_xy.x), y(_xy.y), z(_z) {}
	vector3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
	template <typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
	vector3(const vector3<U>& _other) : x(static_cast<T>(_other.x)), y(static_cast<T>(_other.y)), z(static_cast<T>(_other.z)) {}
	template <typename U>
	vector3(const vector4<U>& _other) : x(static_cast<T>(_other.x)), y(static_cast<T>(_other.y)), z(static_cast<T>(_other.z)) {}

	T& operator[](size_t _i) {
		return *(&x + _i);
	}

	const T& operator[](size_t _i) const {
		return *(&x + _i);
	}

	vector3 operator/(const T& _value) const {
		return {x / _value, y / _value, z / _value};
	}

	vector3 operator*(const T& _value) const {
		return {x * _value, y * _value, z * _value};
	}

	vector3 operator+(const vector3& _value) const {
		return {x + _value.x, y + _value.y, z + _value.z};
	}

	vector3 operator-(const vector3& _value) const {
		return {x - _value.x, y - _value.y, z - _value.z};
	}

	bool operator==(const vector3& _other) const {
		return x == _other.x && y == _other.y && z == _other.z;
	}

	bool operator!=(const vector3& _other) const {
		return !(*this == _other);
	}
};

template <typename T>
vector3<T> operator*(const T& _scalar, const vector3<T>& _value) {
	return {_value.x * _scalar, _value.y * _scalar, _value.z * _scalar};
}

template <typename T>
T dot(const vector3<T>& _left, const vector3<T>& _right) {
	return _left.x * _right.x + _left.y * _right.y + _left.z * _right.z;
}

template <typename T>
vector3<T> cross(const vector3<T>& _left, const vector3<T>& _right) {
	return vector3<T>(
	    _left.y * _right.z - _right.y * _left.z,
	    _left.z * _right.x - _right.z * _left.x,
	    _left.x * _right.y - _right.x * _left.y);
}

template <typename T>
T length(const vector3<T>& _value) {
	return sqrt(dot(_value, _value));
}

template <typename T>
T length_sq(const vector3<T>& _value) {
	return dot(_value, _value);
}

template <typename T>
vector3<T> normalize(const vector3<T>& _value) {
	T len = length(_value);
	if (len > 0)
		return _value / len;
	return {};
}

template <typename T>
vector3<T> normalize_or(const vector3<T>& _value, const vector3<T>& _fallback) {
	T len_sq = length_sq(_value);
	if (len_sq > 0)
		return _value / static_cast<T>(sqrt(len_sq));
	return _fallback;
}

template <typename T>
T radians(T _degrees) {
	return _degrees * (std::numbers::pi / 180.0);
}
} // namespace mars

namespace std {
template <typename T>
struct hash<mars::vector3<T>> {
	size_t operator()(const mars::vector3<T>& _value) const {
		const size_t hx = std::hash<T>{}(_value.x);
		const size_t hy = std::hash<T>{}(_value.y);
		const size_t hz = std::hash<T>{}(_value.z);

		size_t result = hx;
		result ^= hy + 0x9e3779b9 + (result << 6) + (result >> 2);
		result ^= hz + 0x9e3779b9 + (result << 6) + (result >> 2);
		return result;
	}
};
} // namespace std

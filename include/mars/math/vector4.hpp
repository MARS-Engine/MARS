#pragma once

#include "vector2.hpp"

namespace mars {
template <typename T>
struct vector4 {
	T x, y, z, w;

	constexpr vector4() = default;
	constexpr vector4(const T& _value) : x(_value), y(_value), z(_value), w(_value) {}
	constexpr vector4(const T& _x, const T& _y, const T& _z, const T& _w) : x(_x), y(_y), z(_z), w(_w) {}
	constexpr vector4(vector2<T> _xy, vector2<T> _zw) : x(_xy.x), y(_xy.y), z(_zw.x), w(_zw.y) {}
	constexpr vector4(vector2<T> _xy, const T& _z, const T& _w) : x(_xy.x), y(_xy.y), z(_z), w(_w) {}

	vector2<T>& xy() { return *reinterpret_cast<vector2<T>*>(&x); }
	vector2<T>& zw() { return *reinterpret_cast<vector2<T>*>(&z); }

	const vector2<T>& xy() const { return *reinterpret_cast<vector2<T>*>(&x); }
	const vector2<T>& zw() const { return *reinterpret_cast<vector2<T>*>(&z); }

	vector4 operator*(const T& _value) const {
		return {x * _value, y * _value, z * _value, w * _value};
	}

	vector4 operator+(const vector4& _other) const {
		return {x + _other.x, y + _other.y, z + _other.z, w + _other.w};
	}

	vector4& operator/=(const T& _value) {
		x /= _value;
		y /= _value;
		z /= _value;
		w /= _value;
		return *this;
	}

	T& operator[](size_t _i) {
		return *(&x + _i);
	}

	const T& operator[](size_t _i) const {
		return *(&x + _i);
	}
};

template <typename T>
vector4<T> operator*(const T& _scalar, const vector4<T>& _value) {
	return {_value.x * _scalar, _value.y * _scalar, _value.z * _scalar, _value.w * _scalar};
}

template <typename T>
T dot(const vector4<T>& _left, const vector4<T>& _right) {
	return _left.x * _right.x + _left.y * _right.y + _left.z * _right.z + _left.w * _right.w;
}

template <typename T>
T length_sq(const vector4<T>& _value) {
	return dot(_value, _value);
}

template <typename T>
vector4<T> normalize_or(const vector4<T>& _value, const vector4<T>& _fallback) {
	const T len_sq = length_sq(_value);
	if (len_sq > T(0)) {
		vector4<T> result = _value;
		result /= static_cast<T>(sqrt(len_sq));
		return result;
	}
	return _fallback;
}

} // namespace mars

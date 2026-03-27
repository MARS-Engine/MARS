#pragma once

#include "annotation.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
#include <cmath>
#include <mars/container/array.hpp>
#include <numbers>

namespace mars {

template <typename T>
struct[[= mars::matrix(4, 4)]] matrix4 {
	array<vector4<T>, 4> columns = {};

	matrix4() = default;

	matrix4(const T& _value) {
		columns[0, 0] = columns[1, 1] = columns[2, 2] = columns[3, 3] = _value;
	}

	matrix4(const T& _scale_x, const T& _scale_y, const T& _scale_z) {
		columns[0, 0] = _scale_x;
		columns[1, 1] = _scale_y;
		columns[2, 2] = _scale_z;
		columns[3, 3] = 1.0f;
	}

	matrix4(array<vector4<T>, 4> _mat) : columns(_mat) {}

	vector4<T> operator*(const vector4<T>& _value) const {
		return _value[0] * columns[0] +
			   _value[1] * columns[1] +
			   _value[2] * columns[2] +
			   _value[3] * columns[3];
	}

	matrix4 operator*(const matrix4& _right) const {
		matrix4 result;
		for (int y = 0; y < 4; ++y) {
			const vector4<T>& right_col = _right.columns[y];
			result.columns[y] =
				right_col[0] * columns[0] +
				right_col[1] * columns[1] +
				right_col[2] * columns[2] +
				right_col[3] * columns[3];
		}
		return result;
	}

	matrix4& operator/=(const T& _value) {
		columns[0] /= _value;
		columns[1] /= _value;
		columns[2] /= _value;
		columns[3] /= _value;
		return *this;
	}

	template <typename... I>
	auto& operator[](I... _index) { return columns[_index...]; }

	template <typename... I>
	const auto& operator[](I... _index) const { return columns[_index...]; }
};

namespace math {

template <typename T>
T rad(T _deg) {
	return _deg * static_cast<T>(std::numbers::pi_v<T>) / static_cast<T>(180);
}

template <typename T>
T deg(T _rad) {
	return _rad * (static_cast<T>(180) / static_cast<T>(std::numbers::pi_v<T>));
}

template <typename T>
inline matrix4<T> scale(const matrix4<T>& _mat, const vector3<T>& _value) {
	return _mat * matrix4<T>(_value.x, _value.y, _value.z);
}

template <typename T>
matrix4<T> ortho(T _left, T _right, T _bottom, T _top, T _near_z, T _far_z) {
	matrix4<T> result(1);
	result[0, 0] = 2.0f / (_right - _left);
	result[1, 1] = 2.0f / (_top - _bottom);
	result[2, 2] = 1.0f / (_far_z - _near_z);

	result[0, 3] = -(_right + _left) / (_right - _left);
	result[1, 3] = -(_top + _bottom) / (_top - _bottom);
	result[2, 3] = -_near_z / (_far_z - _near_z);
	return result;
}

template <typename T>
inline matrix4<T> translate(const matrix4<T>& _mat, const vector3<T>& _value) {
	matrix4<T> result = _mat;
	result[0, 3] += _value.x;
	result[1, 3] += _value.y;
	result[2, 3] += _value.z;
	return result;
}

template <typename T>
matrix4<T> perspective_fov(T _fovy, T _width, T _height, T _z_near, T _z_far) {
	matrix4<T> result(T(0));

	T const rad = _fovy * static_cast<T>(std::numbers::pi_v<T>) / static_cast<T>(180);
	T const h = cos(static_cast<T>(0.5) * rad) / sin(static_cast<T>(0.5) * rad);
	T const w = h * _height / _width;

	result[0, 0] = w;
	result[1, 1] = h;
	result[3, 2] = static_cast<T>(1);

	result[2, 2] = _z_far / (_z_far - _z_near);
	result[2, 3] = -(_z_far * _z_near) / (_z_far - _z_near);

	return result;
}

template <typename T>
matrix4<T> perspective_fov_reversed_z(T _fovy, T _width, T _height, T _z_near, T _z_far) {
	matrix4<T> result(T(0));

	T const rad = _fovy * static_cast<T>(std::numbers::pi_v<T>) / static_cast<T>(180);
	T const h = cos(static_cast<T>(0.5) * rad) / sin(static_cast<T>(0.5) * rad);
	T const w = h * _height / _width;

	result[0, 0] = w;
	result[1, 1] = h;
	result[3, 2] = static_cast<T>(1);

	result[2, 2] = _z_near / (_z_near - _z_far);
	result[2, 3] = (_z_near * _z_far) / (_z_far - _z_near);

	return result;
}

template <typename T>
matrix4<T> look_at(const vector3<T>& _eye, const vector3<T>& _center, const vector3<T>& _up) {
	vector3<T> const f = normalize(_center - _eye);
	vector3<T> const s = normalize(cross(_up, f));
	vector3<T> const u = cross(f, s);

	matrix4<T> result(1.0f);
	result[0, 0] = s.x;
	result[0, 1] = s.y;
	result[0, 2] = s.z;
	result[1, 0] = u.x;
	result[1, 1] = u.y;
	result[1, 2] = u.z;
	result[2, 0] = f.x;
	result[2, 1] = f.y;
	result[2, 2] = f.z;
	result[0, 3] = -dot(s, _eye);
	result[1, 3] = -dot(u, _eye);
	result[2, 3] = -dot(f, _eye);
	return result;
}

template <typename T>
matrix4<T> rotate(const matrix4<T>& _mat, T _angle, const vector3<T>& _v) {
	T const a = _angle;
	T const c = cos(a);
	T const s = sin(a);

	vector3<T> axis(normalize(_v));
	vector3<T> temp((T(1) - c) * axis);

	matrix4<T> rotate;
	rotate[0, 0] = c + temp[0] * axis[0];
	rotate[0, 1] = temp[0] * axis[1] + s * axis[2];
	rotate[0, 2] = temp[0] * axis[2] - s * axis[1];

	rotate[1, 0] = temp[1] * axis[0] - s * axis[2];
	rotate[1, 1] = c + temp[1] * axis[1];
	rotate[1, 2] = temp[1] * axis[2] + s * axis[0];

	rotate[2, 0] = temp[2] * axis[0] + s * axis[1];
	rotate[2, 1] = temp[2] * axis[1] - s * axis[0];
	rotate[2, 2] = c + temp[2] * axis[2];

	matrix4<T> result;
	result[0] = _mat[0] * rotate[0, 0] + _mat[1] * rotate[0, 1] + _mat[2] * rotate[0, 2];
	result[1] = _mat[0] * rotate[1, 0] + _mat[1] * rotate[1, 1] + _mat[2] * rotate[1, 2];
	result[2] = _mat[0] * rotate[2, 0] + _mat[1] * rotate[2, 1] + _mat[2] * rotate[2, 2];
	result[3] = _mat[3];
	return result;
}

template <typename T>
matrix4<T> transpose(const matrix4<T>& _mat) {
	matrix4<T> result(static_cast<T>(0));
	for (size_t col = 0; col < 4; ++col)
		for (size_t row = 0; row < 4; ++row)
			result[col, row] = _mat[row, col];
	return result;
}

template <typename T>
matrix4<T> inverse(const matrix4<T>& _mat) {
	T inverse_values[16] = {};
	inverse_values[0] = _mat[1, 1] * _mat[2, 2] * _mat[3, 3] - _mat[1, 1] * _mat[2, 3] * _mat[3, 2] - _mat[2, 1] * _mat[1, 2] * _mat[3, 3] + _mat[2, 1] * _mat[1, 3] * _mat[3, 2] + _mat[3, 1] * _mat[1, 2] * _mat[2, 3] - _mat[3, 1] * _mat[1, 3] * _mat[2, 2];
	inverse_values[4] = -_mat[1, 0] * _mat[2, 2] * _mat[3, 3] + _mat[1, 0] * _mat[2, 3] * _mat[3, 2] + _mat[2, 0] * _mat[1, 2] * _mat[3, 3] - _mat[2, 0] * _mat[1, 3] * _mat[3, 2] - _mat[3, 0] * _mat[1, 2] * _mat[2, 3] + _mat[3, 0] * _mat[1, 3] * _mat[2, 2];
	inverse_values[8] = _mat[1, 0] * _mat[2, 1] * _mat[3, 3] - _mat[1, 0] * _mat[2, 3] * _mat[3, 1] - _mat[2, 0] * _mat[1, 1] * _mat[3, 3] + _mat[2, 0] * _mat[1, 3] * _mat[3, 1] + _mat[3, 0] * _mat[1, 1] * _mat[2, 3] - _mat[3, 0] * _mat[1, 3] * _mat[2, 1];
	inverse_values[12] = -_mat[1, 0] * _mat[2, 1] * _mat[3, 2] + _mat[1, 0] * _mat[2, 2] * _mat[3, 1] + _mat[2, 0] * _mat[1, 1] * _mat[3, 2] - _mat[2, 0] * _mat[1, 2] * _mat[3, 1] - _mat[3, 0] * _mat[1, 1] * _mat[2, 2] + _mat[3, 0] * _mat[1, 2] * _mat[2, 1];
	inverse_values[1] = -_mat[0, 1] * _mat[2, 2] * _mat[3, 3] + _mat[0, 1] * _mat[2, 3] * _mat[3, 2] + _mat[2, 1] * _mat[0, 2] * _mat[3, 3] - _mat[2, 1] * _mat[0, 3] * _mat[3, 2] - _mat[3, 1] * _mat[0, 2] * _mat[2, 3] + _mat[3, 1] * _mat[0, 3] * _mat[2, 2];
	inverse_values[5] = _mat[0, 0] * _mat[2, 2] * _mat[3, 3] - _mat[0, 0] * _mat[2, 3] * _mat[3, 2] - _mat[2, 0] * _mat[0, 2] * _mat[3, 3] + _mat[2, 0] * _mat[0, 3] * _mat[3, 2] + _mat[3, 0] * _mat[0, 2] * _mat[2, 3] - _mat[3, 0] * _mat[0, 3] * _mat[2, 2];
	inverse_values[9] = -_mat[0, 0] * _mat[2, 1] * _mat[3, 3] + _mat[0, 0] * _mat[2, 3] * _mat[3, 1] + _mat[2, 0] * _mat[0, 1] * _mat[3, 3] - _mat[2, 0] * _mat[0, 3] * _mat[3, 1] - _mat[3, 0] * _mat[0, 1] * _mat[2, 3] + _mat[3, 0] * _mat[0, 3] * _mat[2, 1];
	inverse_values[13] = _mat[0, 0] * _mat[2, 1] * _mat[3, 2] - _mat[0, 0] * _mat[2, 2] * _mat[3, 1] - _mat[2, 0] * _mat[0, 1] * _mat[3, 2] + _mat[2, 0] * _mat[0, 2] * _mat[3, 1] + _mat[3, 0] * _mat[0, 1] * _mat[2, 2] - _mat[3, 0] * _mat[0, 2] * _mat[2, 1];
	inverse_values[2] = _mat[0, 1] * _mat[1, 2] * _mat[3, 3] - _mat[0, 1] * _mat[1, 3] * _mat[3, 2] - _mat[1, 1] * _mat[0, 2] * _mat[3, 3] + _mat[1, 1] * _mat[0, 3] * _mat[3, 2] + _mat[3, 1] * _mat[0, 2] * _mat[1, 3] - _mat[3, 1] * _mat[0, 3] * _mat[1, 2];
	inverse_values[6] = -_mat[0, 0] * _mat[1, 2] * _mat[3, 3] + _mat[0, 0] * _mat[1, 3] * _mat[3, 2] + _mat[1, 0] * _mat[0, 2] * _mat[3, 3] - _mat[1, 0] * _mat[0, 3] * _mat[3, 2] - _mat[3, 0] * _mat[0, 2] * _mat[1, 3] + _mat[3, 0] * _mat[0, 3] * _mat[1, 2];
	inverse_values[10] = _mat[0, 0] * _mat[1, 1] * _mat[3, 3] - _mat[0, 0] * _mat[1, 3] * _mat[3, 1] - _mat[1, 0] * _mat[0, 1] * _mat[3, 3] + _mat[1, 0] * _mat[0, 3] * _mat[3, 1] + _mat[3, 0] * _mat[0, 1] * _mat[1, 3] - _mat[3, 0] * _mat[0, 3] * _mat[1, 1];
	inverse_values[14] = -_mat[0, 0] * _mat[1, 1] * _mat[3, 2] + _mat[0, 0] * _mat[1, 2] * _mat[3, 1] + _mat[1, 0] * _mat[0, 1] * _mat[3, 2] - _mat[1, 0] * _mat[0, 2] * _mat[3, 1] - _mat[3, 0] * _mat[0, 1] * _mat[1, 2] + _mat[3, 0] * _mat[0, 2] * _mat[1, 1];
	inverse_values[3] = -_mat[0, 1] * _mat[1, 2] * _mat[2, 3] + _mat[0, 1] * _mat[1, 3] * _mat[2, 2] + _mat[1, 1] * _mat[0, 2] * _mat[2, 3] - _mat[1, 1] * _mat[0, 3] * _mat[2, 2] - _mat[2, 1] * _mat[0, 2] * _mat[1, 3] + _mat[2, 1] * _mat[0, 3] * _mat[1, 2];
	inverse_values[7] = _mat[0, 0] * _mat[1, 2] * _mat[2, 3] - _mat[0, 0] * _mat[1, 3] * _mat[2, 2] - _mat[1, 0] * _mat[0, 2] * _mat[2, 3] + _mat[1, 0] * _mat[0, 3] * _mat[2, 2] + _mat[2, 0] * _mat[0, 2] * _mat[1, 3] - _mat[2, 0] * _mat[0, 3] * _mat[1, 2];
	inverse_values[11] = -_mat[0, 0] * _mat[1, 1] * _mat[2, 3] + _mat[0, 0] * _mat[1, 3] * _mat[2, 1] + _mat[1, 0] * _mat[0, 1] * _mat[2, 3] - _mat[1, 0] * _mat[0, 3] * _mat[2, 1] - _mat[2, 0] * _mat[0, 1] * _mat[1, 3] + _mat[2, 0] * _mat[0, 3] * _mat[1, 1];
	inverse_values[15] = _mat[0, 0] * _mat[1, 1] * _mat[2, 2] - _mat[0, 0] * _mat[1, 2] * _mat[2, 1] - _mat[1, 0] * _mat[0, 1] * _mat[2, 2] + _mat[1, 0] * _mat[0, 2] * _mat[2, 1] + _mat[2, 0] * _mat[0, 1] * _mat[1, 2] - _mat[2, 0] * _mat[0, 2] * _mat[1, 1];

	T determinant = _mat[0, 0] * inverse_values[0] + _mat[0, 1] * inverse_values[4] + _mat[0, 2] * inverse_values[8] + _mat[0, 3] * inverse_values[12];
	if (std::abs(determinant) <= static_cast<T>(1e-8))
		return matrix4<T>(static_cast<T>(1));

	determinant = static_cast<T>(1) / determinant;
	matrix4<T> inverse_matrix(static_cast<T>(0));
	for (size_t row = 0; row < 4; ++row)
		for (size_t col = 0; col < 4; ++col)
			inverse_matrix[row, col] = inverse_values[row * 4 + col] * determinant;

	return inverse_matrix;
}
} // namespace math
} // namespace mars

#pragma once

#include "annotation.hpp"
#include "matrix4.hpp"
#include "vector4.hpp"

#include <mars/container/array.hpp>

namespace mars {

template <typename T>
struct [[= mars::matrix(3, 4)]] row_matrix3x4 {
	array<vector4<T>, 3> rows = {};

	row_matrix3x4() = default;

	row_matrix3x4(const matrix4<T>& _matrix) {
		for (size_t row = 0; row < 3; ++row)
			for (size_t col = 0; col < 4; ++col)
				rows[row][col] = _matrix[col, row];
	}

	row_matrix3x4(array<vector4<T>, 3> _rows) : rows(_rows) {}

	template <typename... I>
	auto& operator[](I... _index) { return rows[_index...]; }

	template <typename... I>
	const auto& operator[](I... _index) const { return rows[_index...]; }
};
} // namespace mars

#ifndef __MVRE__MATRIX__BASE__
#define __MVRE__MATRIX__BASE__

#include "math_concept.hpp"
#include "vector_base.hpp"
#include <cstddef>
#include <cstring>

namespace mvre_math {

    /**
     * Column-Major Matrix
     * @tparam T Matrix type, must be arithmetic
     * @tparam Rows Number of Rows
     * @tparam Cols Number of Cols
     */
    template<typename T, size_t Cols, size_t Rows> requires arithmetic<T> class matrix_base {
    protected:
        vector_base<T, Rows> m_data[Cols];
    public:

        matrix_base() = default;

        matrix_base(const matrix_base<T, Cols, Rows>& _vals) {
            memcpy(m_data, _vals.m_data, sizeof(T) * Rows * Cols);
        }


        explicit matrix_base(vector_base<T, Rows>* _vals) {
            memcpy(m_data, _vals, sizeof(T) * Rows * Cols);
        }

        inline vector_base<T, Rows>& get(size_t _col) { return m_data[_col]; }
        inline T get(size_t _col, size_t _row) const { return m_data[_col].get(_row); }
        inline void set(size_t _col, size_t _row, T value) { m_data[_col][_row] = value; }

        inline void set(size_t _col, vector_base<T, Rows> _value) {
            memcpy(&m_data[_col][0], &_value[0], sizeof(T) * Rows);
        }

        inline vector_base<T, Rows>& operator[](size_t _col) {
            return &m_data[_col];
        }

        matrix_base<T, Cols, Rows> operator*(const matrix_base<T, Cols, Rows>& right) const {
            vector_base<T, Rows> new_data[Cols];

            for (auto col = 0; col < Cols; col++)
                for (auto j = 0; j < Rows; j++)
                    for (auto i = 0; i < Cols; i++)
                        new_data[i] +=  m_data[i] * right.get(col, Rows);

            return matrix_base<T, Cols, Rows>(new_data);
        }
    };
}

#endif
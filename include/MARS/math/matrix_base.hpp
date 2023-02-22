#ifndef MARS_MATRIX_BASE_
#define MARS_MATRIX_BASE_

#include "math_concept.hpp"
#include "vector_base.hpp"
#include <cstddef>
#include <cstring>

namespace mars_math {

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

        inline vector_base<T, Rows> get(size_t _col) const { return m_data[_col]; }
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

            int cur_col;
            for (auto col = 0; col < Cols; col++) {
                cur_col = 0;
                for (auto j = 0; j < Rows; j++)
                    new_data[col] +=  m_data[cur_col++] * right.get(col, j);
            }

            return matrix_base<T, Cols, Rows>(new_data);
        }

        void operator*=(const matrix_base<T, Cols, Rows>& right) {
            vector_base<T, Rows> new_data[Cols];

            int cur_col;
            for (auto col = 0; col < Cols; col++) {
                cur_col = 0;
                for (auto j = 0; j < Rows; j++)
                    new_data[col] +=  m_data[cur_col++] * right.get(col, j);
            }

            memcpy(&m_data, &new_data, sizeof(T) * Cols * Rows);
        }

        bool operator==(const matrix_base<T, Cols, Rows>& _right) const {
            return std::memcmp(&m_data, &_right.m_data, sizeof(m_data)) == 0;
        }

        bool operator!=(const matrix_base<T, Cols, Rows>& _right) const {
            return std::memcmp(&m_data, &_right.m_data, sizeof(m_data)) != 0;
        }
    };
}

#endif
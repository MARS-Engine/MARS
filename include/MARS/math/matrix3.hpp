#ifndef MARS_MATRIX3_
#define MARS_MATRIX3_

#include "matrix4.hpp"
#include "vector3.hpp"

namespace mars_math {

    template<typename T> requires arithmetic<T> class matrix3 {
    public:
        vector3<T> col0;
    private:
        T _padding0;
    public:
        vector3<T> col1;
    private:
        T _padding1;
    public:
    vector3<T> col2;
    private:
        T _padding2;
    public:

        matrix3() = default;

        explicit matrix3(const matrix4<T>& _mat) {
            (*this)[0][0] = _mat[0][0];
            (*this)[1][1] = _mat[1][1];
            (*this)[2][2] = _mat[2][2];
        }

        matrix3& inverse() {
            T OneOverDeterminant = static_cast<T>(1) / (
                    + (*this)[0][0] * ((*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2])
                    - (*this)[1][0] * ((*this)[0][1] * (*this)[2][2] - (*this)[2][1] * (*this)[0][2])
                    + (*this)[2][0] * ((*this)[0][1] * (*this)[1][2] - (*this)[1][1] * (*this)[0][2]));

            matrix3<T> inverse;
            inverse[0][0] = + ((*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2]) * OneOverDeterminant;
            inverse[1][0] = - ((*this)[1][0] * (*this)[2][2] - (*this)[2][0] * (*this)[1][2]) * OneOverDeterminant;
            inverse[2][0] = + ((*this)[1][0] * (*this)[2][1] - (*this)[2][0] * (*this)[1][1]) * OneOverDeterminant;
            inverse[0][1] = - ((*this)[0][1] * (*this)[2][2] - (*this)[2][1] * (*this)[0][2]) * OneOverDeterminant;
            inverse[1][1] = + ((*this)[0][0] * (*this)[2][2] - (*this)[2][0] * (*this)[0][2]) * OneOverDeterminant;
            inverse[2][1] = - ((*this)[0][0] * (*this)[2][1] - (*this)[2][0] * (*this)[0][1]) * OneOverDeterminant;
            inverse[0][2] = + ((*this)[0][1] * (*this)[1][2] - (*this)[1][1] * (*this)[0][2]) * OneOverDeterminant;
            inverse[1][2] = - ((*this)[0][0] * (*this)[1][2] - (*this)[1][0] * (*this)[0][2]) * OneOverDeterminant;
            inverse[2][2] = + ((*this)[0][0] * (*this)[1][1] - (*this)[1][0] * (*this)[0][1]) * OneOverDeterminant;

            *this = inverse;

            return *this;
        }

        matrix3& transpose() noexcept {
            matrix3<T> val(*this);

            (*this)[0][0] = val[0][0];
            (*this)[0][1] = val[1][0];
            (*this)[0][2] = val[2][0];
            (*this)[1][0] = val[0][1];
            (*this)[1][1] = val[1][1];
            (*this)[1][2] = val[2][1];
            (*this)[2][0] = val[0][2];
            (*this)[2][1] = val[1][2];
            (*this)[2][2] = val[2][2];

            return *this;
        }

        vector3<T>& operator[](int _index) noexcept {
            switch (_index) {
                case 0:
                    return col0;
                case 1:
                    return col1;
                case 2:
                    return col2;
                default:
                    mars_debug::debug::alert("MARS - MATH - Out of bound index");
                    return col0;
            }
        };

        vector3<T> operator[](int _index) const noexcept {
            switch (_index) {
                case 0:
                    return col0;
                case 1:
                    return col1;
                case 2:
                    return col2;
                default:
                    mars_debug::debug::alert("MARS - MATH - Out of bound index");
                    return col0;
            }
        };
    };
}

#endif
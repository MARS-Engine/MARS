#ifndef MARS_MATRIX3_
#define MARS_MATRIX3_

#include "matrix4.hpp"

namespace mars_math {

    template<typename T> requires arithmetic<T> class matrix3: public matrix_base<T, 3, 4> {
    public:
        matrix3() : matrix_base<T, 3, 4>() { }

        explicit matrix3(matrix_base<T, 4, 4> _mat) {
            this->set(0, _mat.get(0));
            this->set(1, _mat.get(1));
            this->set(2, _mat.get(2));
        }

        matrix3& inverse() {
            T OneOverDeterminant = static_cast<T>(1) / (
                    + this->get(0, 0) * (this->get(1, 1) * this->get(2, 2) - this->get(2, 1) * this->get(1, 2))
                    - this->get(1, 0) * (this->get(0, 1) * this->get(2, 2) - this->get(2, 1) * this->get(0, 2))
                    + this->get(2, 0) * (this->get(0, 1) * this->get(1, 2) - this->get(1, 1) * this->get(0, 2)));

            this->set(0, 0, + (this->get(1, 1) * this->get(2, 2) - this->get(2, 1) * this->get(1, 2)) * OneOverDeterminant);
            this->set(1, 0, - (this->get(1, 0) * this->get(2, 2) - this->get(2, 0) * this->get(1, 2)) * OneOverDeterminant);
            this->set(2, 0, + (this->get(1, 0) * this->get(2, 1) - this->get(2, 0) * this->get(1, 1)) * OneOverDeterminant);
            this->set(0, 1, - (this->get(0, 1) * this->get(2, 2) - this->get(2, 1) * this->get(0, 2)) * OneOverDeterminant);
            this->set(1, 1, + (this->get(0, 0) * this->get(2, 2) - this->get(2, 0) * this->get(0, 2)) * OneOverDeterminant);
            this->set(2, 1, - (this->get(0, 0) * this->get(2, 1) - this->get(2, 0) * this->get(0, 1)) * OneOverDeterminant);
            this->set(0, 2, + (this->get(0, 1) * this->get(1, 2) - this->get(1, 1) * this->get(0, 2)) * OneOverDeterminant);
            this->set(1, 2, - (this->get(0, 0) * this->get(1, 2) - this->get(1, 0) * this->get(0, 2)) * OneOverDeterminant);
            this->set(2, 2, + (this->get(0, 0) * this->get(1, 1) - this->get(1, 0) * this->get(0, 1)) * OneOverDeterminant);

            return *this;
        }

        matrix3& transpose() {
            auto new_data = this->m_data;

            for (auto cols = 0; cols < 3; cols++) {
                for (auto rows = 0; rows < 3; rows++) {
                    new_data[rows][cols] = this->get(cols, rows);
                }
            }
            memcpy(this->m_data, new_data, sizeof(T) * 3 * 3);
            return *this;
        }
    };
}

#endif
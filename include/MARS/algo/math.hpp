#ifndef MARS_MATH_
#define MARS_MATH_

#include <cstddef>
#include <type_traits>

namespace mars_algo {
    template<typename T> T sum(T* _data, size_t _length) {
        size_t total_size = 0;
        for (auto i = 0; i < _length; i++)
            total_size += _data[i];
        return total_size;
    }
}

#endif
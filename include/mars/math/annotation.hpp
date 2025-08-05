#pragma once

#include <cstddef>

namespace mars {
    struct matrix_annotation {
        size_t rows;
        size_t columns;
    };

    static constexpr matrix_annotation matrix(size_t _rows, size_t _columns) { return { _rows, _columns }; }
} // namespace mars
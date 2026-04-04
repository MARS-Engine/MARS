#pragma once
#include <cstdlib>

#if defined(_WIN32)
#include <malloc.h>
#endif

namespace mars {

    inline void* aligned_malloc(size_t _size, size_t _alignment) {
#if defined(_WIN32)
        return _aligned_malloc(_size, _alignment);
#else
        return aligned_alloc(_alignment, _size);
#endif
    }

    inline void aligned_free(void* _ptr) {
#if defined(_WIN32)
        _aligned_free(_ptr);
#else
        free(_ptr);
#endif
    }

}
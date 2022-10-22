#ifndef __MVRE__BASE__BUFFER__
#define __MVRE__BASE__BUFFER__

#include "base_types.hpp"
#include <cstddef>

namespace mvre_graphics_base {

    class base_buffer {
    protected:
        size_t m_size;
        MVRE_MEMORY_TYPE m_mem_type;
    public:
        inline size_t get_size() { return m_size; }

        inline virtual void create(size_t _size, MVRE_MEMORY_TYPE _mem_type) { m_size = _size; m_mem_type = _mem_type; }
        inline virtual void bind() { }
        inline virtual void unbind() { }
        inline virtual void copy_data(void* data) {  }
        inline virtual void clean() {  }
    };
}

#endif
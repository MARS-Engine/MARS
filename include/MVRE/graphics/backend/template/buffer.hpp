#ifndef __MVRE__BUFFER__
#define __MVRE__BUFFER__

#include "graphics_component.hpp"
#include "graphics_types.hpp"
#include <cstddef>

namespace mvre_graphics {

    class buffer : public graphics_component {
    protected:
        size_t m_size;
        MVRE_MEMORY_TYPE m_mem_type;
    public:
        using graphics_component::graphics_component;

        inline size_t size() { return m_size; }

        virtual void create(size_t _size, MVRE_MEMORY_TYPE _mem_type) { m_size = _size; m_mem_type = _mem_type; }

        virtual void bind() { }
        virtual void unbind() { }

        virtual void copy_data(void* data) {  }

        virtual void destroy() {  }
    };
};

#endif
#ifndef MVRE_BUFFER_
#define MVRE_BUFFER_

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

        inline size_t size() const { return m_size; }
        inline MVRE_MEMORY_TYPE type() const { return m_mem_type; }

        virtual void create(size_t _size, MVRE_MEMORY_TYPE _mem_type) { m_size = _size; m_mem_type = _mem_type; }

        virtual void bind() { }
        virtual void unbind() { }

        virtual void copy_data(void* data) {  }

        virtual void destroy() {  }
    };
};

#endif
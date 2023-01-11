#ifndef MARS_BUFFER_
#define MARS_BUFFER_

#include "graphics_component.hpp"
#include "graphics_types.hpp"
#include <cstddef>

namespace mars_graphics {

    class buffer : public graphics_component {
    protected:
        size_t m_size;
        MARS_MEMORY_TYPE m_mem_type;
        void* m_current_data;
    public:
        using graphics_component::graphics_component;

        inline size_t size() const { return m_size; }
        inline MARS_MEMORY_TYPE type() const { return m_mem_type; }

        virtual void create(size_t _size, MARS_MEMORY_TYPE _mem_type) { m_size = _size; m_mem_type = _mem_type; }

        virtual void bind() { }
        virtual void unbind() { }

        inline void copy_data(void* _data) { update(_data); copy_data(); }
        virtual void copy_data() { }
        virtual void update(void* _data) {  }

        virtual void destroy() {  }
    };
};

#endif
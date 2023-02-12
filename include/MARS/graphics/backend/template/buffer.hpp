#ifndef MARS_BUFFER_
#define MARS_BUFFER_

#include "graphics_component.hpp"
#include "graphics_types.hpp"
#include <cstddef>

namespace mars_graphics {

    class buffer : public graphics_component {
    protected:
        size_t m_frames;
        size_t m_size;
        MARS_MEMORY_TYPE m_mem_type;
        void* m_current_data = nullptr;
    public:
        using graphics_component::graphics_component;

        [[nodiscard]] inline size_t size() const { return m_size; }
        [[nodiscard]] inline MARS_MEMORY_TYPE type() const { return m_mem_type; }

        virtual void create(size_t _size, MARS_MEMORY_TYPE _mem_type, size_t _frames) { m_size = _size; m_mem_type = _mem_type; m_frames = _frames; }

        virtual void bind() { }
        virtual void unbind() { }

        virtual void copy_offset(size_t _offset, size_t _size, void* _data) { }
        inline void copy_data(void* _data, size_t _index = 0) { update(_data); copy_data(_index); }
        virtual void copy_data(size_t _index) { }
        virtual void update(void* _data) {
            m_current_data = _data;
        }

        virtual void destroy() {  }
    };
};

#endif
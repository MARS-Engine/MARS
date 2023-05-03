#ifndef MARS_BUFFER_
#define MARS_BUFFER_

#include "builders/buffer_builder.hpp"
#include "graphics_component.hpp"
#include "graphics_types.hpp"
#include <cstddef>

namespace mars_graphics {

    class buffer : public graphics_component {
    protected:
        buffer_data m_data;
        const void* m_current_data = nullptr;

        virtual void create() { }

        inline void set_data(const buffer_data& _data) { m_data = _data; }
    public:
        using graphics_component::graphics_component;

        [[nodiscard]] inline size_t size() const { return m_data.size; }
        [[nodiscard]] inline MARS_MEMORY_TYPE type() const { return m_data.mem_type; }


        virtual void bind() { }
        virtual void unbind() { }

        virtual void copy_offset(size_t _offset, size_t _size, void* _data) { }

        virtual void copy_data(size_t _index) { }

        virtual void update(const void* _data) {
            m_current_data = _data;
        }

        friend buffer_builder;
    };
};

#endif
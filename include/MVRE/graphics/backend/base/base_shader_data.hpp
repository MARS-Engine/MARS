#ifndef __MVRE__BASE__SHADER__DATA__
#define __MVRE__BASE__SHADER__DATA__

#include "base_buffer.hpp"
#include "base_shader.hpp"

#include <vector>

namespace mvre_graphics_base {

    struct base_uniform {
    public:
        
    };

    class base_shader_data {
    protected:
        std::vector<base_buffer*> m_buffer;
    public:
        base_shader* shader;

        virtual void load_from_shader(base_shader* _shader) { shader = _shader; }
    };
}

#endif
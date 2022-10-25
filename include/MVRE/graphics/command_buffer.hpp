#ifndef __COMMAND__BUFFER__
#define __COMMAND__BUFFER__

#include <MVRE/graphics/backend/base/base_command_buffer.hpp>
#include <MVRE/graphics/backend/base/backend_base.hpp>

namespace mvre_graphics {

    class command_buffer : public mvre_graphics_base::backend_base {
    private:
        mvre_graphics_base::base_command_buffer* raw_command_buffer = nullptr;
    public:
        using mvre_graphics_base::backend_base::backend_base;

        void load() override;
        inline void draw(int count, int size) { raw_command_buffer->draw(count, size); }
    };
}

#endif
#ifndef __MVRE__BASE__COMMAND__BUFFER__
#define __MVRE__BASE__COMMAND__BUFFER__

namespace mvre_graphics_base {

    class base_command_buffer {
    public:
        virtual void draw(int count, int size) { }
    };
}

#endif
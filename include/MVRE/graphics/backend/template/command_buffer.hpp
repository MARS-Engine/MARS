#ifndef __MVRE__COMMAND__BUFFER__
#define __MVRE__COMMAND__BUFFER__

namespace mvre_graphics {

    class command_buffer {
    public:
        virtual void draw(int first, size_t _count) { }
        virtual void draw_indexed(size_t _indices) { }
    };
}

#endif
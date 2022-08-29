#ifndef __BUFFER__
#define __BUFFER__

#include <string>

class command_buffer;
class vbuffer;
class vengine;

#include "Vulkan/vtypes.hpp"

enum buffer_type {
    MEM_BUFF_VERTEX,
    MEM_BUFF_INDEX
};

class buffer {
public:
    vbuffer* staging_buffer;
    vbuffer* base_buffer;
    vengine* engine;
    buffer_type type;

    buffer(vengine* _engine);

    void create(size_t _size, buffer_type _type);
    void update(void* _data) const;
    void bind(command_buffer* _command_buffer) const;
};

#endif
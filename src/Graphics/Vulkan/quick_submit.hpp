#ifndef __QUICK__SUBMIT__
#define __QUICK__SUBMIT__

#include "vtypes.hpp"

class vcommand_buffer;
class vcommand_pool;
class vdevice;

class quick_submit {
public:
    VkFence raw_fence;
    vcommand_pool* command_pool;
    vcommand_buffer* command_buffer;
    vdevice* device;

    quick_submit(vdevice* _device);

    void begin();
    void end();
    void clean();
};

#endif
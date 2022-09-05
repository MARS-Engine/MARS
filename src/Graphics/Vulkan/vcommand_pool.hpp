#ifndef __VCOMMAND__POOL__
#define __VCOMMAND__POOL__

#include "vtypes.hpp"

class vdevice;

class vcommand_pool {
public:
    VkCommandPool raw_command_pool;
    vdevice* device;

    vcommand_pool(vdevice* _device);

    void create();
    void clean();
};


#endif
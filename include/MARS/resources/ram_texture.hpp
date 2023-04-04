#ifndef MARS_RAM_TEXTURE_
#define MARS_RAM_TEXTURE_

#include "resource_manager.hpp"

namespace mars_resources {
    class ram_texture : public resource_base {
    private:
        void* m_data;
        
    public:
        bool load_resource(const std::string &_path) override;
        void clean() override;
    };
}

#endif
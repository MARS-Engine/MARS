#ifndef MARS_GLOBAL_BUFFERS_
#define MARS_GLOBAL_BUFFERS_

#include "buffer.hpp"
#include <pl/safe_map.hpp>

namespace mars_graphics {

    class global_buffers {
    private:
        pl::safe_map<std::string, std::shared_ptr<buffer>> m_global_buffers;
    public:
        inline void add_buffer(const std::string& _name, const std::shared_ptr<buffer>& _buffer) {
            m_global_buffers.lock()->insert(std::pair(_name, _buffer));
        }

        inline std::shared_ptr<buffer> get_buffer(const std::string& _name) const {
            return m_global_buffers.lock()->at(_name);
        }

        void destroy();
    };
}

#endif
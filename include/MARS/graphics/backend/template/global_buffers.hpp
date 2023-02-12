#ifndef MARS_GLOBAL_BUFFERS_
#define MARS_GLOBAL_BUFFERS_

#include "buffer.hpp"

namespace mars_graphics {

    class global_buffers {
    private:
        pl::safe_map<std::string, buffer*> m_global_buffers;
    public:
        inline void add_buffer(const std::string& _name, buffer* _buffer) {
            m_global_buffers.lock();
            m_global_buffers.insert(std::pair(_name, _buffer));
            m_global_buffers.unlock();
        }

        inline buffer* get_buffer(const std::string& _name) {
            return m_global_buffers[_name];
        }

        void destroy();
    };
}

#endif
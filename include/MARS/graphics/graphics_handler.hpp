#ifndef MARS_GRAPHICS_HANDLER_
#define MARS_GRAPHICS_HANDLER_

#include <MARS/multithreading/multi_worker.hpp>
#include <MARS/memory/mars_ref.hpp>

namespace mars_graphics {
    class graphics_handler;
    class graphics_engine;

    enum MARS_GRAPHICS_WORKER_TYPE {
        MARS_GRAPHICS_WORKER_TYPE_MULTI_THREAD,
        MARS_GRAPHICS_WORKER_TYPE_SINGLE_THREAD
    };

    class graphics_handler {
    private:
        MARS_GRAPHICS_WORKER_TYPE m_worker_type = MARS_GRAPHICS_WORKER_TYPE_SINGLE_THREAD;
        graphics_engine* m_engine = nullptr;

        void worker_thread();
    public:
        mars_mt::multi_worker workers;

        [[nodiscard]] mars_ref<graphics_engine> engine();
        explicit graphics_handler(graphics_engine* _engine, MARS_GRAPHICS_WORKER_TYPE _type, size_t _active) : workers(_active, [&] { worker_thread(); }) {
            m_engine = _engine;
        }

        inline void execute() {
            workers.execute();
        }

        inline void wait() {
            workers.wait();
        }
    };
}

#endif

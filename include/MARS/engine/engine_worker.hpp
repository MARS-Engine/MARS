#ifndef MARS_ENGINE_WORKER_
#define MARS_ENGINE_WORKER_

#include <atomic>
#include <thread>
#include <semaphore>

namespace mars_engine {

    class engine_handler;

    class engine_worker {
    private:
        engine_handler* m_engine = nullptr;

        std::jthread m_thread;

        std::atomic<bool> m_running = true;
        std::binary_semaphore m_semaphore {0};

        void work();
    public:
        inline void execute() {
            m_semaphore.release();
        }

        inline void wait() {
            m_semaphore.acquire();
        }

        explicit engine_worker(engine_handler* _engine) : m_engine(_engine), m_thread(&engine_worker::work, this) { }
    };
}

#endif
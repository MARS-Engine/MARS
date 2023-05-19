#ifndef MARS_MULTI_WORKER_
#define MARS_MULTI_WORKER_

#include <condition_variable>
#include <latch>
#include <functional>
#include <barrier>

namespace mars_mt {

    class multi_worker {
    private:
        std::function<void()> m_work;

        size_t m_workers = 1;
        std::shared_ptr<std::thread[]> m_threads;
        std::barrier<std::__empty_completion>  m_barrier;

        std::condition_variable m_workers_cv;
        std::condition_variable m_wait_cv;

        std::atomic<bool> m_running;
        std::atomic<bool> m_executing;

        inline void work() {
            while (true) {
                while (!m_executing)
                    m_executing.wait(false);

                if (!m_running)
                    break;

                m_work();

                m_barrier.arrive_and_wait();
                m_executing.exchange(false);
                m_executing.notify_all();
            }
        }
    public:
        multi_worker(size_t _workers, const std::function<void()>& _work) : m_running(true), m_barrier(_workers), m_work(_work) {
            m_workers = _workers;

            m_threads = std::make_unique<std::thread[]>(m_workers);
            for (long i = 0; i < m_workers; i++)
                m_threads[i] = std::thread(&multi_worker::work, this);
        }

        ~multi_worker() {
            m_running.exchange(false);
            wait();
            execute();
            for (long i = 0; i < m_workers; i++)
                m_threads[i].join();
        }

        void execute() {
            m_executing.exchange(true);
            m_executing.notify_all();
        }

        void wait() {
            while (m_executing)
                m_executing.wait(true);
        }
    };
}

#endif
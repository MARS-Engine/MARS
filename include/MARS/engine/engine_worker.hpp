#ifndef MARS_ENGINE_WORKER_
#define MARS_ENGINE_WORKER_

#include <atomic>
#include <memory>
#include <deque>
#include <thread>
#include <condition_variable>
#include <barrier>
#include <typeindex>
#include "object_engine.hpp"

namespace mars_engine {

    class engine_worker : public std::enable_shared_from_this<engine_worker> {
    private:
        std::mutex m_mtx;
        std::condition_variable m_cv;

        size_t m_cores;
        std::atomic<bool> m_running = true;
        std::atomic<long> m_index;
        std::atomic<bool> m_working = false;

        std::barrier<std::function<void()>> m_barriers;

        mars_ref<object_engine> m_engine;
        std::deque<std::jthread> m_threads;
        std::type_index m_layer = typeid(engine_worker);

        inline void on_finish() {
            if (m_engine->get_layer(m_layer)->m_single_time)
                m_engine->clear_layer(m_layer);

            {
                std::lock_guard<std::mutex> l(m_mtx);
                m_working = !m_working;
            }
            m_cv.notify_all();
        }

        void work();
    public:

        inline std::shared_ptr<engine_worker> get_ptr() { return shared_from_this(); }

        engine_worker(const mars_ref<object_engine>& _engine, size_t _cores) : m_cores(_cores), m_engine(_engine), m_barriers(_cores, [&]() { on_finish(); }) {
            m_running = true;

            for (size_t i = 0; i < _cores; i++)
                m_threads.emplace_back(&engine_worker::work, this);
        }

        template<typename T> engine_worker& process_layer() {
            wait();
            m_layer = typeid(T);
            m_engine->get_layer(m_layer)->m_tick.exec_tick();
            m_index = 0;

            {
                std::lock_guard<std::mutex> l(m_mtx);
                m_working = true;
            }

            m_cv.notify_all();
            return *this;
        }

        engine_worker& wait() {
            while (m_working) {
                std::unique_lock<std::mutex> l(m_mtx);
                m_cv.wait(l, [&](){ return !m_working.load(); });
            }

            return *this;
        }

        engine_worker& close() {
            {
                std::lock_guard<std::mutex> l(m_mtx);
                m_running = false;
                m_working = true;
            }

            m_cv.notify_all();
            return *this;
        }

        void join() {
            for (auto& thread : m_threads)
                thread.join();
        }
    };
}

#endif
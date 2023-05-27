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

    enum ENGINE_WORKER_EXECUTION_TYPE {
        ENGINE_WORKER_EXECUTION_TYPE_LAYER,
        ENGINE_WORKER_EXECUTION_TYPE_FUNCTION
    };

    struct engine_worker_exec_layer {
        ENGINE_WORKER_EXECUTION_TYPE type;
        std::type_index layer;
        std::function<void()> function;
        std::function<bool()> should_execute;
    };

    enum ENGINE_WORK_STATUS {
        WORK_STATUS_NONE,
        WORK_STATUS_WAIT,
        WORK_STATUS_EXIT
    };

    class engine_worker : public std::enable_shared_from_this<engine_worker> {
    private:
        size_t m_cores;
        size_t m_execute_index = 0;
        std::atomic<size_t> m_index;
        std::barrier<std::function<void()>> m_barriers;
        bool wait_close = false;
        bool m_exit = false;

        mars_ref<object_engine> m_engine;
        std::deque<std::jthread> m_threads;

        engine_layers* m_active_layer = nullptr;
        std::shared_ptr<std::vector<engine_layer_component>> m_active_components;

        std::vector<engine_worker_exec_layer> m_execution_order;

        inline void get_next() {
            m_index = 0;

            while (m_execution_order[m_execute_index].type == ENGINE_WORKER_EXECUTION_TYPE_FUNCTION) {
                m_execution_order[m_execute_index].function();

                if (++m_execute_index >= m_execution_order.size())
                    m_execute_index = 0;
            }

            m_active_layer = m_engine->get_layer(m_execution_order[m_execute_index].layer);
            m_active_components = m_engine->get_components(m_execution_order[m_execute_index].layer);
        }

        inline void on_finish() {
            if (wait_close)
                m_exit = true;
            m_active_layer->m_tick.exec_tick();

            if (m_active_layer->m_single_time)
                m_engine->clear_layer(m_execution_order[m_execute_index].layer);

            m_execute_index++;

            get_next();
        }

        void work(int i);
    public:

        inline std::shared_ptr<engine_worker> get_ptr() { return shared_from_this(); }

        engine_worker(const mars_ref<object_engine>& _engine, size_t _cores) : m_cores(_cores), m_engine(_engine), m_barriers(_cores, [&]() { on_finish(); }) { }

        template<typename T> engine_worker& add_layer(const std::function<bool()>& _should_execute = nullptr) {
            m_execution_order.push_back({
                .type = ENGINE_WORKER_EXECUTION_TYPE_LAYER,
                .layer = typeid(T),
                .should_execute = _should_execute
            });
            return *this;
        }

        engine_worker& add_function(const std::function<void()>& _function, const std::function<bool()>& _should_execute = nullptr) {
            m_execution_order.push_back({
                .type = ENGINE_WORKER_EXECUTION_TYPE_FUNCTION,
                .layer = typeid(void),
                .function = _function,
                .should_execute = _should_execute,
            });
            return *this;
        }

        void run() {
            get_next();

            for (size_t i = 0; i < m_cores; i++)
                m_threads.emplace_back(&engine_worker::work, this, i);
        }

        void close() {
            wait_close = true;
            for (auto& thread : m_threads)
                thread.join();
        }
    };
}

#endif
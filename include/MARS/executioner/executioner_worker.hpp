#ifndef MARS_EXECUTIONER_WORKER_
#define MARS_EXECUTIONER_WORKER_

#include <pl/safe_map.hpp>
#include <pl/safe.hpp>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <semaphore>

namespace mars_graphics {
    class pipeline;
}

namespace mars_executioner {

    struct executioner_job {
    private:
        mars_graphics::pipeline* m_pipeline = nullptr;
        executioner_job* m_next = nullptr;
        std::mutex m_mtx;
    public:
        inline void lock() { m_mtx.lock(); }
        inline void unlock() { m_mtx.unlock(); }

        inline executioner_job* next() { return m_next; }
        inline void set_next(executioner_job* _next) { m_next = _next; }

        [[nodiscard]] inline executioner_job* get_final() {
            auto final = this;

            while (final->next() != nullptr)
                final = final->next();

            return final;
        }

        std::atomic<bool> started = false;
        std::atomic<bool> finished = false;

        std::function<void()> callback;

        inline void finish() {
            started = false;
            finished = true;
            finished.notify_all();
        }

        inline void reset() {
            started = false;
            finished = false;
            m_next = nullptr;
        }

        inline mars_graphics::pipeline* get_pipeline() { return m_pipeline; }
        inline void change_pipeline(mars_graphics::pipeline* _pipeline) { m_pipeline = _pipeline; }

        explicit executioner_job(mars_graphics::pipeline* _pipeline, const std::function<void()>& _callback) { m_pipeline = _pipeline; finished = false; callback = _callback; }
        explicit executioner_job(const std::function<void()>& _callback) { m_pipeline = nullptr; finished = false; callback = _callback; }

        void wait() {
            finished.wait(false);
        }

        void clean() { finished = false; }
    };

    class executioner_worker {
    private:
        std::atomic<bool> m_running = false;
        std::atomic<bool> m_execute = false;
        std::binary_semaphore m_semaphore {0};

        pl::safe_map<mars_graphics::pipeline*, pl::safe<executioner_job*>> render_jobs;

        std::thread _thread;
        std::mutex gpu_lock;

    public:
        std::mutex job_mtx;

        inline void lock_gpu() {
            gpu_lock.lock();
        }

        inline void unlock_gpu() {
            gpu_lock.unlock();
        }

        inline void stop() { m_running = false; m_semaphore.release(); }

        inline bool executing() { return m_execute; }

        inline void execute() {
            m_execute.exchange(true);
            m_semaphore.release();
        };

        inline void add_job(executioner_job* _job) {
            if (_job->get_pipeline() == nullptr)
                return;

            if (!render_jobs.contains(_job->get_pipeline())) {
                render_jobs.lock();
                render_jobs.insert(std::pair(_job->get_pipeline(), nullptr));
                render_jobs.unlock();
            }

            auto job = render_jobs[_job->get_pipeline()].lock_get();

            if (job == nullptr) {
                render_jobs[_job->get_pipeline()].set(_job);
                render_jobs[_job->get_pipeline()].unlock();
                return;
            }

            render_jobs[_job->get_pipeline()].unlock();

            auto tail = job->get_final();
            tail->lock();

            while (tail->next() != nullptr) {
                tail->unlock();
                tail = tail->get_final();
                tail->lock();
            }

            tail->set_next(_job);
            tail->unlock();
        }

        executioner_worker();
        ~executioner_worker();
        void worker();
    };
}

#endif
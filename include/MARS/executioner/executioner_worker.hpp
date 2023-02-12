#ifndef MARS_EXECUTIONER_WORKER_
#define MARS_EXECUTIONER_WORKER_

#include <pl/safe_map.hpp>
#include <pl/safe_vector.hpp>
#include <functional>
#include <condition_variable>
#include <atomic>

namespace mars_graphics {
    class pipeline;
}

namespace mars_executioner {

    enum EXECUTIONER_JOB_PRIORITY {
        EXECUTIONER_JOB_PRIORITY_NORMAL,
        EXECUTIONER_JOB_PRIORITY_IN_FLIGHT
    };

    struct executioner_job {
    private:
        mars_graphics::pipeline* m_pipeline;
    public:
        std::atomic<bool> started = false;
        std::atomic<bool> finished = false;

        std::mutex mtx;
        ///condition_variable called when finished
        std::condition_variable wait_room;
        std::function<void()> callback;

        inline void finish() {
            started = false;
            finished = true;
        }

        inline void reset() {
            std::unique_lock<std::mutex> l(mtx);
            started = false;
            finished = false;
        }

        inline mars_graphics::pipeline* get_pipeline() { return m_pipeline; }
        inline void change_pipeline(mars_graphics::pipeline* _pipeline) { m_pipeline = _pipeline; }

        explicit executioner_job(mars_graphics::pipeline* _pipeline, const std::function<void()>& _callback) { m_pipeline = _pipeline; finished = false; callback = _callback; }
        explicit executioner_job(const std::function<void()>& _callback) { m_pipeline = nullptr; finished = false; callback = _callback; }

        void wait() {
            std::unique_lock<std::mutex> l(mtx);
            wait_room.wait(l, [&]() { return finished.load(); });
        }

        void clean() { finished = false; }
    };

    class executioner_worker {
    private:
        std::atomic<bool> m_running = false;
        std::atomic<bool> m_execute = false;
        std::condition_variable m_worker_cv;

        pl::safe_map<EXECUTIONER_JOB_PRIORITY, pl::safe_vector<executioner_job*>> m_jobs;
        pl::safe_map<mars_graphics::pipeline*, pl::safe_vector<executioner_job*>> render_jobs;

        std::thread _thread;
        std::mutex gpu_lock;

    public:
        std::mutex job_mtx;
        std::mutex render_mtx;
        std::condition_variable wait_room;

        inline void lock_gpu() {
            gpu_lock.lock();
        }

        inline void unlock_gpu() {
            gpu_lock.unlock();
        }

        inline void stop() { m_running = false; m_worker_cv.notify_all(); }

        inline bool executing() { return m_execute; }

        inline void execute() {
            {
                std::lock_guard lk(job_mtx);
                m_execute = true;
            }
            m_worker_cv.notify_all();
        };

        inline void add_job(EXECUTIONER_JOB_PRIORITY _priority, executioner_job* _job) {
            if (_priority == EXECUTIONER_JOB_PRIORITY_NORMAL && _job->get_pipeline() != nullptr) {
                {
                    std::unique_lock lk(render_mtx);
                    render_jobs.lock();
                    render_jobs[_job->get_pipeline()].push_back(_job);
                    render_jobs.unlock();
                }
            }
            else {
                {
                    std::unique_lock lk(job_mtx);
                    m_jobs.lock();
                    m_jobs[_priority].push_back(_job);
                    m_jobs.unlock();
                }

                if (_priority == EXECUTIONER_JOB_PRIORITY_IN_FLIGHT)
                    m_worker_cv.notify_all();
            }
        }

        executioner_worker();
        ~executioner_worker();
        void worker();
    };
}

#endif
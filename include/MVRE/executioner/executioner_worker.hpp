#ifndef __MVRE__EXECUTIONER__WORKER__
#define __MVRE__EXECUTIONER__WORKER__

#include <pl/safe_map.hpp>
#include <pl/safe_vector.hpp>
#include <functional>
#include <condition_variable>
#include <atomic>

namespace mvre_executioner {

    enum EXECUTIONER_JOB_PRIORITY {
        EXECUTIONER_JOB_PRIORITY_NORMAL,
        EXECUTIONER_JOB_PRIORITY_IN_FLIGHT
    };

    struct executioner_job {
    public:
        std::atomic<bool> finished;

        std::mutex mtx;
        ///condition_variable called when finished
        std::condition_variable wait_room;
        std::function<void()> callback;

        inline void reset() {
            std::unique_lock<std::mutex> l(mtx);
            finished = false;
        }

        explicit executioner_job(std::function<void()> _callback) { finished = false; callback = _callback; }

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

        std::thread _thread;

    public:
        std::mutex mtx;
        std::condition_variable wait_room;

        inline void stop() { m_running = false; m_worker_cv.notify_all(); }

        inline bool executing() { return m_execute; }

        inline void execute() {
            {
                std::lock_guard lk(mtx);
                m_execute = true;
            }
            m_worker_cv.notify_all();
        };

        inline void add_job(EXECUTIONER_JOB_PRIORITY _priority, executioner_job* _job) {
            {
                std::unique_lock lk(mtx);
                m_jobs[_priority].push_back(_job);
            }
            m_worker_cv.notify_all();
        }

        executioner_worker();
        ~executioner_worker();
        void worker();
    };
}

#endif
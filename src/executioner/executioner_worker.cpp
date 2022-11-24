#include "MVRE/executioner/executioner_worker.hpp"
#include <mutex>

using namespace mvre_executioner;

executioner_worker::executioner_worker() {
    m_running = true;
    _thread = std::thread(&executioner_worker::worker, this);
}

executioner_worker::~executioner_worker() {
    if (m_running)
        stop();
    _thread.join();
}

void executioner_worker::worker() {

    while (m_running) {
        //1ms just to ensure that the code doesn't lock, temp solution

        {
            std::unique_lock lk(mtx);
            m_worker_cv.wait(lk, [&]{ return !m_jobs[EXECUTIONER_JOB_PRIORITY_IN_FLIGHT].empty() || m_execute || !m_running; });
        }

        while (!m_jobs[EXECUTIONER_JOB_PRIORITY_IN_FLIGHT].empty()) {
            auto job = m_jobs[EXECUTIONER_JOB_PRIORITY_IN_FLIGHT][0];
            job->callback();
            {
                std::lock_guard lk(job->mtx);
                job->finished = true;
            }
            job->wait_room.notify_all();
            m_jobs[EXECUTIONER_JOB_PRIORITY_IN_FLIGHT].erase_at(0);
        }

        if (m_execute) {
            while (!m_jobs[EXECUTIONER_JOB_PRIORITY_NORMAL].empty()) {
                auto job = m_jobs[EXECUTIONER_JOB_PRIORITY_NORMAL][0];
                job->callback();
                {
                    std::lock_guard lk(job->mtx);
                    job->finished = true;
                }
                job->wait_room.notify_all();
                m_jobs[EXECUTIONER_JOB_PRIORITY_NORMAL].erase_at(0);
            }
        }

        {
            std::lock_guard lk(mtx);
            m_execute = false;
        }
        wait_room.notify_all();
    }
}
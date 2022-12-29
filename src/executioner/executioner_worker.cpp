#include <MVRE/executioner/executioner_worker.hpp>
#include <MVRE/graphics/backend/template/pipeline.hpp>
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
        {
            std::unique_lock lk(job_mtx);
            m_worker_cv.wait(lk, [&]{ return !m_jobs[EXECUTIONER_JOB_PRIORITY_IN_FLIGHT].empty() || m_execute || !m_running; });
        }

        while (!m_jobs[EXECUTIONER_JOB_PRIORITY_IN_FLIGHT].empty()) {
            m_jobs.lock();
            auto job = m_jobs[EXECUTIONER_JOB_PRIORITY_IN_FLIGHT][0];
            m_jobs[EXECUTIONER_JOB_PRIORITY_IN_FLIGHT].erase_at(0);
            m_jobs.unlock();

            job->callback();
            {
                std::lock_guard lk(job->mtx);
                job->finished = true;
            }
            job->wait_room.notify_all();
        }

        if (m_execute) {
            while (!m_jobs[EXECUTIONER_JOB_PRIORITY_NORMAL].empty()) {
                m_jobs.lock();
                auto job = m_jobs[EXECUTIONER_JOB_PRIORITY_NORMAL][0];
                m_jobs[EXECUTIONER_JOB_PRIORITY_NORMAL].erase_at(0);
                m_jobs.unlock();

                job->callback();
                {
                    std::lock_guard lk(job->mtx);
                    job->finished = true;
                }
                job->wait_room.notify_all();
            }

            render_jobs.lock();
            for (auto& pair : render_jobs) {
                pair.first->bind();
                while (!render_jobs[pair.first].empty()) {
                    auto job = render_jobs[pair.first][0];
                    job->callback();
                    {
                        std::lock_guard lk(job->mtx);
                        job->finished = true;
                    }
                    job->wait_room.notify_all();
                    render_jobs[pair.first].erase_at(0);
                }
            }
            render_jobs.unlock();
        }

        {
            std::lock_guard lk(job_mtx);
            m_execute = false;
        }
        wait_room.notify_all();
    }
}
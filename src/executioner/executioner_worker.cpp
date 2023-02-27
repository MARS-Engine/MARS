#include <MARS/executioner/executioner_worker.hpp>
#include <MARS/graphics/backend/template/pipeline.hpp>

using namespace mars_executioner;

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
        m_semaphore.acquire();

        if (m_execute) {
            for (auto& pair : render_jobs) {
                pair.first->bind();
                auto head = render_jobs[pair.first].get();

                while (head != nullptr) {
                    head->started = true;
                    head->callback();
                    head->finish();
                    head = head->next();
                }

                render_jobs[pair.first].lock();
                render_jobs[pair.first].set(nullptr);
                render_jobs[pair.first].unlock();
            }
        }

        m_execute.exchange(false);
    }
}
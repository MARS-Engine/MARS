#ifndef MARS_EXECUTIONER_
#define MARS_EXECUTIONER_

#include "executioner_worker.hpp"


namespace mars_executioner {

    class executioner {
    private:
        static executioner_worker* worker;
    public:
        static inline void lock_gpu() {
            worker->lock_gpu();
        }

        static inline void unlock_gpu() {
            worker->unlock_gpu();
        }

        static inline void add_job(EXECUTIONER_JOB_PRIORITY _priority, executioner_job* _job) { _job->reset();  worker->add_job(_priority, _job); }

        static inline bool finished() { return !worker->executing(); }

        static inline void stop() { worker->stop(); }

        static void init();
        static void execute();
        static void clean();
    };
}

#endif
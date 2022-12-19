#ifndef MVRE_EXECUTIONER_
#define MVRE_EXECUTIONER_

#include "executioner_worker.hpp"


namespace mvre_executioner {

    class executioner {
    private:
        static executioner_worker* worker;
    public:
        static inline void add_job(EXECUTIONER_JOB_PRIORITY _priority, executioner_job* _job) { _job->reset();  worker->add_job(_priority, _job); }

        inline static void stop() { worker->stop(); }

        static void init();
        static void execute();
        static void clean();
    };
}

#endif
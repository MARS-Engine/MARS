#ifndef __MVRE__GL__ENGINE__INSTANCE__
#define __MVRE__GL__ENGINE__INSTANCE__

#include "MVRE/graphics/backend/base/base_engine_instance.hpp"
#include "MVRE/executioner/executioner.hpp"

namespace mvre_graphics_opengl {
    class gl_engine_instance : public mvre_graphics_base::base_engine_instance {
        private:
            mvre_executioner::executioner_job* clear_job;
            mvre_executioner::executioner_job* swap_job;
        public:

            void create_with_window(mvre_graphics::window& _window) override;
            void update() override;
            void clear() override;
            void clean() override;
    };
}

#endif
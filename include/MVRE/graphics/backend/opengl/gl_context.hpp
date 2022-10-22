#ifndef __MVRE__GL__CONTEXT__
#define __MVRE__GL__CONTEXT__

#include "MVRE/graphics/backend/base/base_context.hpp"

namespace mvre_graphics_opengl {

    class gl_context : public mvre_graphics_base::base_context {
    public:
        using mvre_graphics_base::base_context::base_context;

        SDL_GLContext raw_context;

        void create() override;
        void clean() override;
    };
}

#endif
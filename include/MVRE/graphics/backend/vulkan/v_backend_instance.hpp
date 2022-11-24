#ifndef __MVRE__V__BACKEND__INSTANCE__
#define __MVRE__V__BACKEND__INSTANCE__

#include <MVRE/graphics/backend/template/backend_instance.hpp>

namespace mvre_graphics {

    class v_backend_instance : public backend_instance {
        using backend_instance::backend_instance;

        void create_with_window(const std::string& _title, mvre_math::vector2<int> _size) override;

        void update() override;
        void prepare_render() override;
        void draw() override;
        void destroy() override;
    };
}

#endif
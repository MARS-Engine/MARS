#ifndef MVRE_GRAPHICS_INSTANCE_
#define MVRE_GRAPHICS_INSTANCE_

#include "backend/template/backend_instance.hpp"
#include <MVRE/math/matrix4.hpp>

namespace mvre_graphics {


    class camera {
    private:
        mvre_math::matrix4<float> view;
        mvre_math::matrix4<float> projection;
        mvre_math::matrix4<float> view_proj;
    public:
        inline void set_view(const mvre_math::matrix4<float>& _val) { view = _val; }
        inline void set_projection(const mvre_math::matrix4<float>& _val) { projection = _val; }
        inline void set_view_proj(const mvre_math::matrix4<float>& _val) { view_proj = _val; }

        inline mvre_math::matrix4<float> get_view() { return view; }
        inline mvre_math::matrix4<float> get_projection() { return projection; }
        inline mvre_math::matrix4<float> get_view_proj() { return view_proj; }
    };

    class graphics_instance {
    private:
        backend_instance* m_instance = nullptr;
        camera m_camera;
    public:
        inline camera& get_camera() { return m_camera; }
        inline bool is_running() { return !m_instance->get_window()->should_close(); }

        template<typename T> inline T* instance() { return m_instance->instance<T>(); }

        inline backend_instance* backend() { return m_instance; }

        inline mvre_graphics::command_buffer* primary_buffer() { return  m_instance->primary_buffer(); }

        explicit graphics_instance(backend_instance* _instance) {
            m_instance = _instance;
        }

        void create_with_window(const std::string& _title, mvre_math::vector2<int> _size) {
            m_instance->create_with_window(_title, _size);
        }

        void update() {
            m_instance->update();
        }

        void prepare_render() {
            m_instance->prepare_render();
        }

        void draw() {
            m_instance->draw();
        }

        void destroy() {
            m_instance->destroy();
        }
    };
}

#endif
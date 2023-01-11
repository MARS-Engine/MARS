#ifndef MARS_GRAPHICS_INSTANCE_
#define MARS_GRAPHICS_INSTANCE_

#include "backend/template/backend_instance.hpp"
#include <MARS/math/matrix4.hpp>
#include <MARS/input/input_manager.hpp>

namespace mars_graphics {


    class camera {
    private:
        mars_math::matrix4<float> view;
        mars_math::matrix4<float> projection;
        mars_math::matrix4<float> view_proj;
        mars_math::vector3<float> m_position;
    public:
        inline mars_math::vector3<float>& position() { return m_position; }
        inline void position(const mars_math::vector3<float>& _position) { m_position = _position; }

        inline void set_view(const mars_math::matrix4<float>& _val) { view = _val; }
        inline void set_projection(const mars_math::matrix4<float>& _val) { projection = _val; }
        inline void set_view_proj(const mars_math::matrix4<float>& _val) { view_proj = _val; }

        inline mars_math::matrix4<float> get_view() { return view; }
        inline mars_math::matrix4<float> get_projection() { return projection; }
        inline mars_math::matrix4<float> get_proj_view() { return view_proj; }
    };

    class graphics_instance {
    private:
        backend_instance* m_instance = nullptr;
        camera m_camera;
        mars_input::input* m_input = nullptr;

    public:
        inline camera& get_camera() { return m_camera; }
        inline bool is_running() { return !m_instance->get_window()->should_close(); }
        inline size_t current_frame() { return m_instance->current_frame(); }

        template<typename T> inline T* instance() { return m_instance->instance<T>(); }

        inline backend_instance* backend() { return m_instance; }

        inline mars_graphics::command_buffer* primary_buffer() { return  m_instance->primary_buffer(); }

        inline void window_update() {
            m_instance->get_window()->process(m_input);
        }

        explicit graphics_instance(backend_instance* _instance) {
            m_instance = _instance;
            m_instance->set_instance(this);
        }

        void create_with_window(const std::string& _title, mars_math::vector2<int> _size) {
            m_instance->create_with_window(_title, _size);
            m_input = mars_input::input_manager::create_input(m_instance->get_window());
        }

        void update() {
            m_instance->update();
            m_input->update();
        }

        void finish_update() {
            m_input->finish_update();
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
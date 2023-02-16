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

        inline mars_math::matrix4<float> get_view() const { return view; }
        inline mars_math::matrix4<float> get_projection() const { return projection; }
        inline mars_math::matrix4<float> get_proj_view() const { return view_proj; }
    };

    class graphics_instance {
    private:
        backend_instance* m_instance = nullptr;
        camera m_camera;
        mars_input::input* m_input = nullptr;

    public:
        [[nodiscard]] inline camera& get_camera() { return m_camera; }
        [[nodiscard]] inline bool is_running() const { return !m_instance->get_window()->should_close(); }
        [[nodiscard]] inline size_t current_frame() const { return m_instance->current_frame(); }

        template<typename T> [[nodiscard]] inline T* instance() const { return m_instance->instance<T>(); }

        [[nodiscard]] inline backend_instance* backend() const { return m_instance; }

        [[nodiscard]] inline mars_graphics::command_buffer* primary_buffer() const { return  m_instance->primary_buffer(); }

        [[nodiscard]] inline std::string render_type() const { return m_instance->render_type(); }

        inline void window_update() {
            m_instance->get_window()->process(m_input);
        }

        explicit graphics_instance(backend_instance* _instance) {
            m_instance = _instance;
            m_instance->set_instance(this);
        }

        inline void create_with_window(const std::string& _title, const mars_math::vector2<size_t>& _size, const std::string& _renderer) {
            m_instance->create_with_window(_title, _size, _renderer);
            m_input = mars_input::input_manager::create_input(m_instance->get_window());
        }

        inline void update() {
            m_instance->update();
            m_input->update();
        }

        inline void finish_update() {
            m_input->finish_update();
        }

        inline void prepare_render() {
            m_instance->prepare_render();
        }

        inline void draw() {
            m_instance->draw();
        }

        inline void destroy() {
            m_instance->destroy();
        }

        inline void wait_idle() {
            m_instance->wait_idle();
        }
    };
}

#endif
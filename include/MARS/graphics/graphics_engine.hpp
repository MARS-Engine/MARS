#ifndef MARS_GRAPHICS_INSTANCE_
#define MARS_GRAPHICS_INSTANCE_

#include <functional>
#include "backend/template/graphics_backend.hpp"
#include <MARS/math/matrix4.hpp>
#include <MARS/input/input_manager.hpp>
#include "graphics_handler.hpp"
#include "engine_camera.hpp"
#include <pl/vector_ptr.hpp>

namespace mars_graphics {

    enum MARS_PIPELINE_CALLBACK_STATUS {
        MARS_PIPELINE_CALLBACK_STATUS_WAITING,
        MARS_PIPELINE_CALLBACK_STATUS_WORKING,
        MARS_PIPELINE_CALLBACK_STATUS_COMPLETE
    };

    struct graphics_draw_call {
        std::function<void()> draw_call;

        explicit graphics_draw_call(const std::function<void()>& _call) {
            draw_call = _call;
        }
    };

    class graphics_engine : public std::enable_shared_from_this<graphics_engine> {
    private:
        graphics_backend* m_instance = nullptr;
        camera m_camera;
        mars_input::input* m_input = nullptr;
        graphics_handler m_handler;
        pl::vector_ptr<graphics_draw_call> m_drawcalls;
    public:
        std::shared_ptr<graphics_engine> get_ptr() { return shared_from_this(); }

        [[nodiscard]] inline mars_resources::resource_manager* resources() const { return backend()->resources(); }
        [[nodiscard]] inline camera& get_camera() { return m_camera; }
        [[nodiscard]] inline bool is_running() const { return !m_instance->get_window()->should_close(); }
        [[nodiscard]] inline size_t current_frame() const { return m_instance->current_frame(); }

        template<typename T> [[nodiscard]] inline std::shared_ptr<T> create() const { return m_instance->create<T>(); }

        [[nodiscard]] inline graphics_backend* backend() const { return m_instance; }

        [[nodiscard]] inline mars_graphics::command_buffer* primary_buffer() const { return  m_instance->primary_buffer(); }

        [[nodiscard]] inline std::string render_type() const { return m_instance->render_type(); }

        [[nodiscard]] inline pl::vector_ptr<graphics_draw_call>& get_drawcalls() { return m_drawcalls; }

        inline void add_drawcall(const std::function<void()>& _draw_call) {
            auto call = graphics_draw_call(_draw_call);
            m_drawcalls.push_back(call);
        }

        inline void window_update() {
            m_instance->get_window()->process(m_input);
        }

        explicit graphics_engine(graphics_backend* _instance, size_t _threads) : m_handler(this, _threads == 1 ? MARS_GRAPHICS_WORKER_TYPE_SINGLE_THREAD : MARS_GRAPHICS_WORKER_TYPE_MULTI_THREAD, _threads) {
            m_instance = _instance;
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

            m_handler.execute();

        }

        inline void wait_draw() {
            m_handler.wait();
        }

        inline void swap() {
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
#ifndef MVRE_GRAPHICS_INSTANCE_
#define MVRE_GRAPHICS_INSTANCE_

#include "backend/template/backend_instance.hpp"

namespace mvre_graphics {

    struct engine_instance_data {
    public:
        std::string name;
        mvre_math::vector2<int> size;
    };

    class graphics_instance {
    private:
        backend_instance* m_instance = nullptr;
    public:
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
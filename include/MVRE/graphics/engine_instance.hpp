#ifndef __MVRE__ENGINE__INSTANCE__
#define __MVRE__ENGINE__INSTANCE__

#include <string>

#include "MVRE/math/vector2.hpp"

#include "mvre_graphics_types.hpp"
#include "backend/base/base_engine_instance.hpp"

namespace mvre_graphics {

    struct engine_instance_data {
    public:
        std::string name;
        mvre_math::vector2<int> size;
        MVRE_INSTANCE_TYPE type = MVRE_INSTANCE_TYPE_VULKAN;
    };

    class engine_instance {
    private:
        window m_window;
        engine_instance_data m_instance_data;
    public:
        mvre_graphics_base::base_engine_instance* backend;

        inline MVRE_INSTANCE_TYPE get_backend_type() { return m_instance_data.type; }

        inline std::string get_window_name() const { return m_instance_data.name; }
        inline mvre_math::vector2<int> get_window_size() const { return m_instance_data.size; }

        inline bool is_running() { return !m_window.should_close(); };

        explicit engine_instance(engine_instance_data _instance_data) {
            m_instance_data = _instance_data;
        }

        void create_with_window();
        void update() const;
        void prepare_render() const;
        void clean();
    };
}

#endif
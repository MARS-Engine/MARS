#ifndef MARS_TRANSFORM_3D_
#define MARS_TRANSFORM_3D_

#include <MARS/math/matrix4.hpp>
#include <MARS/math/vector3.hpp>
#include <MARS/math/quaternion.hpp>
#include <atomic>

namespace mars_engine {
    class mars_object;


    class transform_3d {
    private:
        bool m_need_update;

        mars_object* m_object = nullptr;

        bool m_enable_scale;
        bool m_enable_rotation;

        std::atomic<mars_math::vector3<float>> m_position;
        std::atomic<mars_math::vector3<float>> m_scale;
        std::atomic<mars_math::quaternion<float>> m_rotation;

        std::atomic<mars_math::matrix4<float>> m_transform_mat;
    public:
        [[nodiscard]] inline mars_math::vector3<float> position() const { return m_position; };
        [[nodiscard]] inline mars_math::vector3<float> scale() const { return m_scale; };
        [[nodiscard]] inline mars_math::quaternion<float> rotation() const { return m_rotation; };
        [[nodiscard]] inline mars_math::matrix4<float> matrix() { if (m_need_update) update(); return m_transform_mat; }

        [[nodiscard]] mars_math::vector3<float> get_world_position() const;

        [[nodiscard]] mars_math::quaternion<float> get_world_rotation() const;

        inline void set_position(const mars_math::vector3<float>& _val) { if (m_position.load() != _val) m_need_update = true; m_position = _val; };
        inline void set_scale(const mars_math::vector3<float>& _val) { if (m_scale.load() != _val) m_need_update = true; m_scale = _val; };
        inline void set_rotation(const mars_math::quaternion<float>& _val) { if (m_rotation.load() != _val) m_need_update = true; m_rotation = _val; };

        inline void set_parent(mars_object* _parent) {
            m_object = _parent;
        }

        transform_3d() {
            m_scale = mars_math::vector3<float>(1);
            m_need_update = true;
            m_enable_scale = true;
            m_enable_rotation = true;
        }

        void update();
    };
}

#endif
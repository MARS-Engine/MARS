#ifndef MARS_TRANSFORM_3D_
#define MARS_TRANSFORM_3D_

#include <MARS/math/vector3.hpp>
#include <MARS/math/quaternion.hpp>
#include "engine_object.hpp"

namespace mars_engine {

    class transform_3d {
    private:
        bool m_need_update;

        engine_object* m_parent;

        bool m_enable_scale;
        bool m_enable_rotation;

        mars_math::vector3<float> m_position;
        mars_math::vector3<float> m_scale;
        mars_math::quaternion<float> m_rotation;

        mars_math::matrix4<float> m_transform_mat;
    public:
        inline mars_math::vector3<float> position() { return m_position; };
        inline mars_math::vector3<float> scale() { return m_scale; };
        inline mars_math::quaternion<float> rotation() { return m_rotation; };
        inline mars_math::matrix4<float> matrix() { if (m_need_update) update(); return m_transform_mat; }

        inline mars_math::vector3<float> get_world_position() {
            engine_object* next_parent = m_parent->parent();
            mars_math::vector3<float> result = position();

            while (next_parent != nullptr) {
                result += m_parent->transform()->position();
                next_parent = m_parent->parent();
            }

            return result;
        }

        inline mars_math::quaternion<float> get_world_rotation() {
            engine_object* next_parent = m_parent->parent();
            mars_math::quaternion<float> result = rotation();

            while (next_parent != nullptr) {
                result *= m_parent->transform()->rotation();
                next_parent = m_parent->parent();
            }

            return result;
        }

        inline void set_position(const mars_math::vector3<float>& _val) { if (m_position != _val) m_need_update = true; m_position = _val; };
        inline void set_scale(const mars_math::vector3<float>& _val) { if (m_scale != _val) m_need_update = true; m_scale = _val; };
        inline void set_rotation(const mars_math::quaternion<float>& _val) { if (m_rotation != _val) m_need_update = true; m_rotation = _val; };

        explicit transform_3d(engine_object* _obj) {
            m_parent = _obj;
            m_scale = mars_math::vector3<float>(1);
            m_need_update = true;
            m_enable_scale = true;
            m_enable_rotation = true;
        }

        void update();
    };
}

#endif
#ifndef __MVRE__TRANSFORM__3D__
#define __MVRE__TRANSFORM__3D__

#include <MVRE/math/vector3.hpp>
#include <MVRE/math/quaternion.hpp>
#include "engine_object.hpp"

namespace mvre_engine {

    class transform_3d {
    private:
        bool m_need_update;

        engine_object* m_parent;

        bool m_enable_scale;
        bool m_enable_rotation;

        mvre_math::vector3<float> m_position;
        mvre_math::vector3<float> m_scale;
        mvre_math::quaternion<float> m_rotation;

        mvre_math::matrix4<float> m_transform_mat;
    public:
        inline mvre_math::vector3<float> position() { return m_position; };
        inline mvre_math::vector3<float> scale() { return m_scale; };
        inline mvre_math::quaternion<float> rotation() { return m_rotation; };
        inline mvre_math::matrix4<float> matrix() { if (m_need_update) update(); return m_transform_mat; }

        inline mvre_math::vector3<float> get_world_position() {
            engine_object* next_parent = m_parent->parent();
            mvre_math::vector3<float> result = position();

            while (next_parent != nullptr) {
                result += m_parent->transform()->position();
                next_parent = m_parent->parent();
            }

            return result;
        }

        inline mvre_math::quaternion<float> get_world_rotation() {
            engine_object* next_parent = m_parent->parent();
            mvre_math::quaternion<float> result = rotation();

            while (next_parent != nullptr) {
                result *= m_parent->transform()->rotation();
                next_parent = m_parent->parent();
            }

            return result;
        }

        inline void set_position(const mvre_math::vector3<float>& _val) { if (m_position != _val) m_need_update = true; m_position = _val; };
        inline void set_scale(const mvre_math::vector3<float>& _val) { if (m_scale != _val) m_need_update = true; m_scale = _val; };
        inline void set_rotation(const mvre_math::quaternion<float>& _val) { if (m_rotation != _val) m_need_update = true; m_rotation = _val; };

        explicit transform_3d(engine_object* _obj) {
            m_parent = _obj;
            m_scale = mvre_math::vector3<float>(1);
            m_need_update = true;
            m_enable_scale = true;
            m_enable_rotation = true;
        }

        void update();
    };
}

#endif
#include <MARS/engine/transform_3d.hpp>
#include <MARS/engine/mars_object.hpp>

using namespace mars_engine;

mars_math::vector3<float> transform_3d::get_world_position() {
    auto next_parent = m_object->parent();
    mars_math::vector3<float> result = position();

    while (next_parent != nullptr) {
        result += m_object->transform().position();
        next_parent = m_object->parent();
    }

return result;
}

mars_math::quaternion<float> transform_3d::get_world_rotation() {
    auto next_parent = m_object->parent();
    mars_math::quaternion<float> result = rotation();

    while (next_parent != nullptr) {
        result *= m_object->transform().rotation();
        next_parent = m_object->parent();
    }

    return result;
}

void transform_3d::update() {
    m_transform_mat = mars_math::matrix4<float>(1);
    m_transform_mat.translate(position());

    if (m_enable_rotation)
        m_transform_mat *= mars_math::matrix4<float>::from_quaternion(rotation());

    if (m_enable_scale)
        m_transform_mat.scale(scale());

    if (m_object->parent() != nullptr)
        m_transform_mat *= m_object->parent()->transform().matrix();

    m_need_update = false;
}
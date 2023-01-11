#include <MARS/engine/transform_3d.hpp>

using namespace mars_engine;

void transform_3d::update() {
    if (!m_need_update)
        return;

    m_transform_mat = mars_math::matrix4<float>(1);
    m_transform_mat.translate(position());

    if (m_enable_rotation)
        m_transform_mat *= mars_math::matrix4<float>::from_quaternion(rotation());

    if (m_enable_scale)
        m_transform_mat.scale(scale());

    if (m_parent->parent() != nullptr)
        m_transform_mat *= m_parent->parent()->transform()->matrix();

    m_need_update = false;
}
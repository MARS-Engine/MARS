#include <MARS/math/vector_base.hpp>
#include <xmmintrin.h>

using namespace mars_math;

template<> vector_base<float, 4> mars_math::vector_base<float, 4>::operator+(const vector_base<float, 4UL> &right) const {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_add_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set_ps(right.m_data[3], right.m_data[2], right.m_data[1], right.m_data[0])));
    return vector_base<float, 4>(result);
}

template<> vector_base<float, 4> mars_math::vector_base<float, 4>::operator-(const vector_base<float, 4UL> &right) const {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_sub_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set_ps(right.m_data[3], right.m_data[2], right.m_data[1], right.m_data[0])));
    return vector_base<float, 4>(result);
}

template<> vector_base<float, 4> mars_math::vector_base<float, 4>::operator*(const vector_base<float, 4UL> &right) const {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_mul_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set_ps(right.m_data[3], right.m_data[2], right.m_data[1], right.m_data[0])));
    return vector_base<float, 4>(result);
}

template<> vector_base<float, 4> mars_math::vector_base<float, 4>::operator*(const float &right) const {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_mul_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set1_ps(right)));
    return vector_base<float, 4>(result);
}


template<> vector_base<float, 4> mars_math::vector_base<float, 4>::operator/(const vector_base<float, 4UL> &right) const {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_div_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set_ps(right.m_data[3], right.m_data[2], right.m_data[1], right.m_data[0])));
    return vector_base<float, 4>(result);
}

template<> vector_base<float, 4> vector_base<float, 4>::operator/(const float &right) const {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_div_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set1_ps(right)));
    return vector_base<float, 4>(result);
}

template<> vector_base<float, 4>& mars_math::vector_base<float, 4>::operator+=(const vector_base<float, 4>& right) {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_add_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set_ps(right.m_data[3], right.m_data[2], right.m_data[1], right.m_data[0])));
    memcpy(m_data, result, sizeof(result));
    return *this;
}

template<> vector_base<float, 4>& mars_math::vector_base<float, 4>::operator-=(const vector_base<float, 4>& right) {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_sub_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set_ps(right.m_data[3], right.m_data[2], right.m_data[1], right.m_data[0])));
    memcpy(m_data, result, sizeof(result));
    return *this;
}

template<> vector_base<float, 4>& mars_math::vector_base<float, 4>::operator*=(const vector_base<float, 4>& right) {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_mul_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set_ps(right.m_data[3], right.m_data[2], right.m_data[1], right.m_data[0])));
    memcpy(m_data, result, sizeof(result));
    return *this;
}

template<> vector_base<float, 4>& mars_math::vector_base<float, 4>::operator/=(const vector_base<float, 4>& right) {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_div_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set_ps(right.m_data[3], right.m_data[2], right.m_data[1], right.m_data[0])));
    memcpy(m_data, result, sizeof(result));
    return *this;
}

template<> vector_base<float, 4>& mars_math::vector_base<float, 4>::operator*=(const float& right) {
    __attribute__ ((aligned(16)))
    float result[4];
    _mm_store_ps(result, _mm_mul_ps(_mm_set_ps(m_data[3], m_data[2], m_data[1], m_data[0]), _mm_set1_ps(right)));
    memcpy(m_data, result, sizeof(result));
    return *this;
}
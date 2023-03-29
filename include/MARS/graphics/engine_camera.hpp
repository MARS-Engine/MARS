#ifndef  MARS_ENGINE_CAMERA_
#define  MARS_ENGINE_CAMERA_

#include <MARS/math/matrix4.hpp>

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

        [[nodiscard]] inline mars_math::matrix4<float> get_view() const { return view; }
        [[nodiscard]] inline mars_math::matrix4<float> get_projection() const { return projection; }
        [[nodiscard]] inline mars_math::matrix4<float> get_proj_view() const { return view_proj; }
    };
}

#endif
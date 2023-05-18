#ifndef MARS_BRIDGE_
#define MARS_BRIDGE_

#include <MARS/memory/mars_ref.hpp>

namespace mars_engine {
    class mars_object;

    class bridge : public std::enable_shared_from_this<bridge> {
    private:
        mars_ref<mars_object> m_object;

        inline void set_object(const mars_ref<mars_object>& _object) {
            m_object = _object;
        }

        friend mars_object;
    public:
        [[nodiscard]] inline mars_ref<mars_object> object() const { return m_object; }
    };
}

#endif
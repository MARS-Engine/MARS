#ifndef MARS_BRIDGE_
#define MARS_BRIDGE_

#include <MARS/memory/mars_ref.hpp>

namespace mars_engine {
    class mars_object;

    class bridge : public std::enable_shared_from_this<bridge> {
    private:
        mars_ref<mars_object> m_object;
    public:
        inline std::shared_ptr<bridge> get_ptr() { return shared_from_this(); }

        [[nodiscard]] inline mars_ref<mars_object> get_engine_object() const { return m_object; }

        explicit bridge(const mars_ref<mars_object>& _object);
    };
}

#endif
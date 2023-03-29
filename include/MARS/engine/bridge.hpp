#ifndef MARS_BRIDGE_
#define MARS_BRIDGE_

#include <memory>

namespace mars_engine {
    class mars_object;

    class bridge : public std::enable_shared_from_this<bridge> {
    private:
        std::shared_ptr<mars_object> m_object;
    public:
        inline std::shared_ptr<bridge> get_ptr() { return shared_from_this(); }

        [[nodiscard]] inline std::shared_ptr<mars_object> get_engine_object() const { return m_object; }

        explicit bridge(mars_object& _object);
    };
}

#endif
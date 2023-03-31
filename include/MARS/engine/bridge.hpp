#ifndef MARS_BRIDGE_
#define MARS_BRIDGE_

#include <memory>

namespace mars_engine {
    class _mars_object;
    using mars_object = std::shared_ptr<_mars_object>;

    class bridge : public std::enable_shared_from_this<bridge> {
    private:
        mars_object m_object;
    public:
        inline std::shared_ptr<bridge> get_ptr() { return shared_from_this(); }

        [[nodiscard]] inline mars_object get_engine_object() const { return m_object; }

        explicit bridge(const mars_object& _object);
    };
}

#endif
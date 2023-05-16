#ifndef MARS_ENGINE_SINGLETON_
#define MARS_ENGINE_SINGLETON_

#include <MARS/memory/mars_ref.hpp>

namespace mars_engine {
    class object_engine;

    class singleton {
    private:
        mars_ref<object_engine> m_engine;

        inline void set_engine(const mars_ref<object_engine>& _engine) {
            m_engine = _engine;
        }

        friend object_engine;
    public:
        [[nodiscard]] inline mars_ref<object_engine> engine() const { return m_engine; }
    };
}

#endif
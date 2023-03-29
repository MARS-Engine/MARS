#ifndef MARS_ENGINE_SINGLETON_
#define MARS_ENGINE_SINGLETON_

#include <memory>

namespace mars_engine {
    class object_engine;

    class singleton {
    private:
        std::shared_ptr<object_engine> m_engine;
    public:
        [[nodiscard]] inline std::shared_ptr<object_engine> engine() const { return m_engine; }

        explicit singleton(std::shared_ptr<object_engine> _engine) { m_engine = _engine; }
    };
}

#endif
#ifndef MARS_ENGINE_SINGLETON_
#define MARS_ENGINE_SINGLETON_

#include <memory>

namespace mars_engine {
    class _object_engine;
    typedef std::shared_ptr<_object_engine> object_engine;

    class singleton {
    private:
        object_engine m_engine;
    public:
        [[nodiscard]] inline object_engine engine() const { return m_engine; }

        explicit singleton(object_engine _engine) { m_engine = _engine; }
    };
}

#endif
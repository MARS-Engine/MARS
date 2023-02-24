#ifndef MARS_ENGINE_SINGLETON_
#define MARS_ENGINE_SINGLETON_

namespace mars_engine {
    class engine_handler;

    class singleton {
    private:
        engine_handler* m_engine;
    public:
        [[nodiscard]] inline engine_handler* engine() const { return m_engine; }

        explicit singleton(engine_handler* _engine) { m_engine = _engine; }
    };
}

#endif
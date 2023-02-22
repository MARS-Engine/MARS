#ifndef MARS_ENGINE_SINGLETON_
#define MARS_ENGINE_SINGLETON_

namespace mars_engine {
    class engine_handler;

    class singleton {
    private:
        engine_handler* m_instance;
    public:
        [[nodiscard]] inline engine_handler* instance() const { return m_instance; }

        explicit singleton(engine_handler* _instance) { m_instance = _instance; }
    };
}

#endif
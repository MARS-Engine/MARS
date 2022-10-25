#ifndef __MVRE__ENGINE__OBJECT__
#define __MVRE__ENGINE__OBJECT__

#include <pl/safe_vector.hpp>

#include "MVRE/graphics/engine_instance.hpp"
#include "engine_handler.hpp"

namespace mvre_engine {

    template<class T> class component;
    class component_interface;

    class engine_object {
    private:
        engine_object* m_parent = nullptr;
        mvre_graphics::engine_instance* m_instance = nullptr;
    public:
        pl::safe_vector<component_interface*> components;

        inline void set_instance(mvre_graphics::engine_instance* _new_instance) { m_instance = _new_instance; }
        inline mvre_graphics::engine_instance* instance() { return m_instance; }

        inline void set_parent(engine_object* _parent) { m_parent = _parent; }

        ~engine_object();

        void execute(MVRE_EXECUTION_CODE _code);

        template<class T> T* get_component() {
            static_assert(std::is_base_of<component<T>, T>::value, "invalid component - component must have type mvre_engine::component has a base");
            for (auto c : components) {
                auto cast = dynamic_cast<T*>(c);
                if (cast != nullptr)
                    return cast;
            }
        }

        template<class T> T* add_component(T* _new_component) {
            static_assert(std::is_base_of<component<T>, T>::value, "invalid component - component must have type mvre_engine::component has a base");
            components.push_back(_new_component);
            _new_component->set_object(this);
            return _new_component;
        }

        template<class T> inline T* add_component() { return add_component<T>(new T()); }
    };
};

#endif
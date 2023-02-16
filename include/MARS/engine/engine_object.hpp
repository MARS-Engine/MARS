#ifndef MARS_ENGINE_OBJECT_
#define MARS_ENGINE_OBJECT_

#include <pl/safe_vector.hpp>
#include <pl/safe.hpp>
#include <MARS/graphics/graphics_instance.hpp>

namespace mars_engine {

    class transform_3d;
    class component;
    class engine_handler;

    class engine_object {
    private:
        engine_object* m_prev = nullptr;
        engine_object* m_next = nullptr;
        engine_object* m_parent = nullptr;
        mars_graphics::graphics_instance* m_instance = nullptr;
        engine_handler* m_engine = nullptr;
        transform_3d* m_transform = nullptr;
        pl::safe<engine_object*> m_children = nullptr;
        pl::safe_vector<component*> m_components;
    public:
        [[nodiscard]] inline engine_object* get_final() {
            auto final = this;

            while (final->next() != nullptr)
                final = final->next();

            return final;
        }

        [[nodiscard]] inline engine_object* next() const { return m_next; }
        [[nodiscard]] inline engine_object* previous() const { return m_prev; }

        inline void set_next(engine_object* _val) { m_next = _val; }

        inline void set_previous(engine_object* _val) { m_prev = _val; }

        [[nodiscard]] inline engine_object* parent() const { return m_parent; }
        [[nodiscard]] inline transform_3d* transform() const { return m_transform; }
        [[nodiscard]] inline pl::safe<engine_object*>& children() { return m_children; }
        [[nodiscard]] inline pl::safe_vector<component*>& components() { return m_components; }
        [[nodiscard]] inline mars_graphics::graphics_instance* instance() const { return m_instance; }
        [[nodiscard]] inline engine_handler* engine() const { return m_engine; }

        void set_instance(mars_graphics::graphics_instance* _new_instance) {
            m_instance = _new_instance;

            //TODO: Make non-recursive
            auto child = children().lock_get();
            while (child != nullptr) {
                child->set_instance(m_instance);
                child = child->next();
            }

            children().unlock();
        }

        void set_engine(engine_handler* _handler) {
            m_engine = _handler;

            //TODO: Make non-recursive
            auto child = children().lock_get();
            while (child != nullptr) {
                child->set_engine(m_engine);
                child = child->next();
            }

            children().unlock();
        }

        inline void set_parent(engine_object* _parent) {
            m_parent = _parent;
            auto parent_child = m_parent->children().lock_get();

            if (parent_child == nullptr)
                m_parent->children() = this;
            else {
                auto tail = parent_child->get_final();
                tail->set_next(this);
                set_previous(tail);
            }

            m_parent->children().unlock();
        }

        engine_object();
        ~engine_object();

        template<class T> T* get_component() const {
            static_assert(std::is_base_of<component, T>::value, "invalid component - component must have type mars_engine::component has a base");
            for (auto c : m_components) {
                auto cast = dynamic_cast<T*>(c);
                if (cast != nullptr)
                    return cast;
            }
        }

        template<class T> T* add_component(T* _new_component) {
            static_assert(std::is_base_of<component, T>::value, "invalid component - component must have type mars_engine::component has a base");
            m_components.push_back(_new_component);
            _new_component->set_object(this);
            return _new_component;
        }

        template<class T> inline T* add_component() { return add_component<T>(new T()); }
    };
};

#endif
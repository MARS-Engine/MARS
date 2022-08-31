#ifndef __ENGINE__OBJECT__
#define __ENGINE__OBJECT__

#include "thread_manager.hpp"
#include "Graphics/vengine.hpp"
#include "Graphics/command_buffer.hpp"
#include "Graphics/material.hpp"
#include "Type/shader_types.hpp"

class transform_3d;
template<class T> class component;
class component_interface;

class engine_object {
private:
    command_buffer* _command_buffer = nullptr;
    vengine* _engine = nullptr;

    bool recursion_checker(engine_object* object);
public:
    int core_id = -1;

    vector<component_interface*>* components;
    vector<engine_object*>* children;

    engine_object* parent = nullptr;
    transform_3d* transform  = nullptr;
    material* mat = nullptr;

    engine_object();
    ~engine_object();

    void execute_code(execution_code code);
    void add_child(engine_object* child);

    inline void set_engine(vengine* engine) {
        _engine = engine;
        for (auto c : *children)
            c->set_engine(engine);
    }

    inline vengine* get_engine() { return _engine; }

    inline command_buffer* get_command_buffer() {
        if (_command_buffer == nullptr) {
            _command_buffer = new command_buffer(get_engine());
            _command_buffer->create();
        }

        return _command_buffer;
    }

    template<class T> T* add_component(T* _component) {
        static_assert(is_base_of<component<T>, T>::value, "Attempted to add a component that doesn't have Component as base");
        components->push_back(_component);
        _component->object = this;
        return _component;
    }

    template<class T> T* add_component() {
        static_assert(is_base_of<component<T>, T>::value, "Attempted to add a component that doesn't have Component as base");
        T* t = new T();
        components->push_back(t);
        t->object = this;
        return t;
    }

    template<class T> T* get_component() {
        for (auto comp : *components) {
            auto conv = dynamic_cast<T*>(comp);
            if (conv != nullptr)
                return conv;
        }

        return nullptr;
    }
};

#endif
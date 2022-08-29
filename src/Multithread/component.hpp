#ifndef __COMPONENT__
#define __COMPONENT__

#include "Graphics/vengine.hpp"
#include "Graphics/command_buffer.hpp"
#include "Components/transform_3d.hpp"

class component_interface {
public:
    bool is_renderer = false;
    bool loaded = false;
    engine_object* object = nullptr;

    virtual size_t size() const;
    virtual void pre_load();
    virtual void load();
    virtual void pre_render();
    virtual void update();
    virtual void render();
    virtual void post_render();
    virtual void clean();
};

template<typename derived> class component : public component_interface {
public:
    inline transform_3d* get_transform() const { return object->transform; };
    inline vengine* get_engine() const { return object->get_engine(); }
    inline material* get_material() const { return object->mat; }
    inline void set_material(material* mat) const { object->mat = mat; }

    size_t size() const override { return sizeof(derived); }

    template<class T> inline T* get_component() { return object->get_component<T>(); }
    template<class T> inline T* add_component(T* comp) { return object->add_component(comp); }
    template<class T> inline T* add_component() { return object->add_component<T>(); }


    inline command_buffer* get_command_buffer() {
        return object->get_command_buffer();
    }
};

#endif
#include "engine_object.hpp"
#include "component.hpp"
#include "Components/transform_3d.hpp"
#include "Manager/render_pass_manager.hpp"
#include "Manager/material_manager.hpp"
#include "Components/Graphics/camera.hpp"

engine_object::engine_object() {
    mat = nullptr;
    transform = new transform_3d(this);
    children = new std::vector<engine_object*>();
    components = new std::vector<component_interface*>();
}

engine_object::~engine_object() {
    for (auto c : *components)
        delete c;

    components->clear();
    delete components;

    for (auto c : *children)
        delete c;

    children->clear();
    delete children;
}

bool engine_object::recursion_checker(engine_object* object) {
    if (object->parent == object)
        return false;
    return object->parent == nullptr || object->parent->recursion_checker(object);
}

void engine_object::execute_code(execution_code code) {
    switch (code) {
        case PRE_RENDER:
            if (find_if(components->begin(), components->end(), [](auto c) { return c->is_renderer; }) == components->end())
                break;

            for (int i = 0; i < vengine::frame_overlap; i++) {
                get_command_buffer()->begin(i);
                get_command_buffer()->load_default(i); //TODO: remove in future or add a conditional
                for (auto& component : *components)
                    component->pre_render();
                get_command_buffer()->end();
            }
            break;
        case RENDER:
            if (_command_buffer == nullptr)
                break;

            if (mat->enable_transparency)
                _engine->trans_queue.push_back({vector3::distance(transform->get_position(),
                                                                  _engine->get_camera()->get_transform()->get_position()),
                                                get_command_buffer()->base_command_buffer->raw_command_buffers[_engine->render_frame]});
            else
                _engine->draw_queue.push_back(get_command_buffer()->base_command_buffer->raw_command_buffers[_engine->render_frame]);
            break;
        default:
            for (auto& component : *components) {
                switch (code) {
                    case PRE_LOAD:
                        component->pre_load();
                        break;
                    case LOAD:
                        component->load();
                        break;
                    case UPDATE:
                        component->update();
                        break;
                    case POST_RENDER:
                        component->post_render();
                        break;
                    case CLEAN:
                        component->clean();
                        break;
                }
            }
            break;
    }

    for (auto& child : *children)
        child->execute_code(code);

    if (code == UPDATE)
        transform->set_has_updated(false);
}

void engine_object::add_child(engine_object* child) {
    if (child == this || !recursion_checker(child))
        return debug::alert("Attempted to create a child that is itself, a parent or parent of parent");

    children->push_back(child);
    child->parent = this;
    child->_engine = _engine;
}
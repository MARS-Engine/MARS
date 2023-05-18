#include <MARS/graphics/graphics_engine.hpp>
#include <MARS/engine/object_engine.hpp>

using namespace mars_graphics;

void graphics_engine::window_update() {
    m_instance->get_window()->process(m_engine->get<mars_input::input>());
}

void graphics_engine::create_with_window(const std::string &_title, const mars_math::vector2<int> &_size, const std::string &_renderer) {
    m_instance->create_with_window(_title, _size, _renderer);
    m_engine->get<mars_input::input>();
}

void graphics_engine::update() {
    m_instance->update();
    m_engine->get<mars_input::input>()->update();
}

void graphics_engine::finish_update() {
    m_engine->get<mars_input::input>()->finish_update();
}
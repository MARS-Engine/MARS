#include <MARS/graphics/graphics_engine.hpp>
#include <MARS/engine/object_engine.hpp>

using namespace mars_graphics;

void graphics_engine::window_update() {
    m_instance.lock()->get_window()->process(m_engine->get_singleton<mars_input::input>());
}

void graphics_engine::create_with_window(const std::string &_title, const mars_math::vector2<int> &_size, const std::string &_renderer) {
    m_instance.lock()->create_with_window(_title, _size, _renderer);
    m_engine->get_singleton<mars_input::input>();
}

void graphics_engine::update() {
    m_instance.lock()->update();
    m_engine->get_singleton<mars_input::input>()->update();
}

void graphics_engine::finish_update() {
    m_engine->get_singleton<mars_input::input>()->finish_update();
}
#include <MARS/graphics/backend/template/backend_instance.hpp>
#include <MARS/graphics/renderer/renderer.hpp>

std::string mars_graphics::backend_instance::render_type() const { return m_renderer->get_render_type(); }
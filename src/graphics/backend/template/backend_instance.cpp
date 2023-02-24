#include <MARS/graphics/backend/template/graphics_backend.hpp>
#include <MARS/graphics/renderer/renderer.hpp>

std::string mars_graphics::graphics_backend::render_type() const { return m_renderer->get_render_type(); }
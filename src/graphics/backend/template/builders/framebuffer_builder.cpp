#include <MARS/graphics/backend/template/builders/framebuffer_builder.hpp>
#include <MARS/graphics/backend/template/framebuffer.hpp>

using namespace mars_graphics;

std::shared_ptr<framebuffer> framebuffer_builder::build(swapchain* _swapchain) {
    base_build();
    m_ref->set_data(m_data);
    m_ref->create(_swapchain);

    return m_ref;
}

std::shared_ptr<framebuffer> framebuffer_builder::build(const std::vector<std::shared_ptr<texture>>& _textures) {
    base_build();
    m_ref->set_data(m_data);
    m_ref->create(_textures);

    return m_ref;
}
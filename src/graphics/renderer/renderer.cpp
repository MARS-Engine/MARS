#include <MARS/graphics/renderer/renderer.hpp>
#include <MARS/resources/resource_manager.hpp>
#include <MARS/algo/string_helper.hpp>

using namespace mars_graphics;
using namespace mars_resources;

std::map<std::string, RENDERER_TEXTURE_TYPE> render_frame_token = {
        { "color",    RENDERER_TEXTURE_TYPE_COLOR },
        { "depth",    RENDERER_TEXTURE_TYPE_DEPTH },
        { "position", RENDERER_TEXTURE_TYPE_POSITION },
        { "swapchain", RENDERER_TEXTURE_TYPE_FORWARD },
        { "#frame",   RENDERER_TEXTURE_TYPE_FRAME }
};

void renderer::create(const std::string& _path) {
    if (_path.empty())
        mars_debug::debug::error("MARS - Renderer - Path Empty");

    auto path_split = mars_string::explode(_path, '.');
    m_render_type = path_split[0];

    std::vector<std::string> _data;

    resource_manager::read_file(graphics()->resources()->find_path(_path, MARS_RESOURCE_TYPE_RENDERER), _data);

    std::string frame_name;
    texture* active_texture;
    bool has_forward = false;
    for (size_t i = 0; auto& line : _data) {
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        auto values = mars_string::explode(line, ' ');

        if (!render_frame_token.contains(values[0]))
            continue;

        auto size = graphics()->get_window()->size();

        switch (render_frame_token[values[0]]) {
            case RENDERER_TEXTURE_TYPE_FRAME:
                if (!frame_name.empty()) {
                    if (has_forward) {
                        if (m_framebuffers.size() != 1)
                            m_framebuffers[frame_name].frame->set_load_previous(true);
                        m_framebuffers[frame_name].frame->create(graphics()->get_swapchain());
                    }
                    else
                        m_framebuffers[frame_name].frame->create(graphics()->get_window()->size(), m_framebuffers[frame_name].buffers);
                }

                has_forward = false;
                if (values.size() > 1)
                    frame_name = values[1];

                m_framebuffers[frame_name] = {};
                m_framebuffers[frame_name].frame = graphics()->create<framebuffer>();
                m_framebuffers[frame_name].frame->set_size(size);

                if (i++ != 0)
                    m_framebuffers[frame_name].frame->set_load_previous(true);
                break;
            case RENDERER_TEXTURE_TYPE_COLOR:
                active_texture = graphics()->create<texture>();
                active_texture->set_size(size);
                active_texture->create(MARS_FORMAT_SBGRA_8, MARS_TEXTURE_USAGE_COLOR);
                m_framebuffers[frame_name].buffers.push_back(active_texture);
                break;
            case RENDERER_TEXTURE_TYPE_POSITION:
                active_texture = graphics()->create<texture>();
                active_texture->set_size(size);
                active_texture->create(MARS_FORMAT_F_RGBA16, MARS_TEXTURE_USAGE_COLOR);
                m_framebuffers[frame_name].buffers.push_back(active_texture);
                break;
            case RENDERER_TEXTURE_TYPE_FORWARD:
                has_forward = true;
                break;
            case RENDERER_TEXTURE_TYPE_DEPTH:
                m_framebuffers[frame_name].frame->set_depth(true);
                break;
        }
    }

    if (has_forward) {
        if (m_framebuffers.size() != 1)
            m_framebuffers[frame_name].frame->set_load_previous(true);
        m_framebuffers[frame_name].frame->create(graphics()->get_swapchain());
    }
    else
        m_framebuffers[frame_name].frame->create(graphics()->get_window()->size(), m_framebuffers[frame_name].buffers);
}

void renderer::destroy() {
    for (auto& frame : m_framebuffers) {
        frame.second.frame->destroy();
        delete frame.second.frame;

        for (auto& texture : frame.second.buffers) {
            texture->clean();
            delete texture;
        }
    }
}
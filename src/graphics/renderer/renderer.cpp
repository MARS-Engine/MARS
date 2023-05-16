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
    auto builder = graphics()->builder<texture_builder>();
    bool has_forward = false;

    auto frame_builder = graphics()->builder<framebuffer_builder>();

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
                            frame_builder.set_load_previous(true);
                        m_framebuffers[frame_name].frame = frame_builder.build(graphics()->get_swapchain());
                    }
                    else {
                        frame_builder.set_size({ static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) });
                        m_framebuffers[frame_name].frame = frame_builder.build(m_framebuffers[frame_name].buffers);
                    }
                }

                has_forward = false;
                if (values.size() > 1)
                    frame_name = values[1];

                m_framebuffers[frame_name] = {};
                frame_builder = graphics()->builder<framebuffer_builder>();
                frame_builder.set_size({ static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) });

                if (i++ != 0)
                    frame_builder.set_load_previous(true);
                break;
            case RENDERER_TEXTURE_TYPE_COLOR:
                builder = graphics()->builder<texture_builder>();
                builder.set_size(size).set_format(MARS_FORMAT_SBGRA_8).set_usage(MARS_TEXTURE_USAGE_COLOR);
                m_framebuffers[frame_name].buffers.push_back(builder.build());
                break;
            case RENDERER_TEXTURE_TYPE_POSITION:
                builder = graphics()->builder<texture_builder>();
                builder.set_size(size).set_format(MARS_FORMAT_F_RGBA16).set_usage(MARS_TEXTURE_USAGE_COLOR);
                m_framebuffers[frame_name].buffers.push_back(builder.build());
                break;
            case RENDERER_TEXTURE_TYPE_FORWARD:
                has_forward = true;
                break;
            case RENDERER_TEXTURE_TYPE_DEPTH:
                frame_builder.set_depth(true);
                break;
        }
    }

    if (has_forward) {
        if (m_framebuffers.size() != 1)
            frame_builder.set_load_previous(true);
        m_framebuffers[frame_name].frame = frame_builder.build(graphics()->get_swapchain());
    }
    else {
        auto size = graphics()->get_window()->size();
        frame_builder.set_size({ static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) });
        m_framebuffers[frame_name].frame = frame_builder.build(m_framebuffers[frame_name].buffers);
    }
}

void renderer::destroy() {
    for (auto& frame : m_framebuffers)
        frame.second.buffers.clear();
    m_framebuffers.clear();
}
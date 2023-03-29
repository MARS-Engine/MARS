#include <MARS/graphics/material.hpp>
#include <MARS/algo/string_helper.hpp>


using namespace mars_graphics;
using namespace mars_resources;

std::map<std::string, MARS_MATERIAL_INPUT> material::mat_input_tokens = {
        { "shader", MARS_MATERIAL_INPUT_SHADER },
        { "texture", MARS_MATERIAL_INPUT_TEXTURE }
};

bool material::load_resource(const std::string &_path) {
    std::vector<std::string> lines;
    if (!resource_manager::read_file(_path, lines))
        mars_debug::debug::error("MARS - Material - Failed to find material - " + _path);

    for (const auto& line : lines) {
        if (!line.contains(':'))
            continue;

        std::vector<std::string> data = mars_string::explode(line, ':');

        if (data.size() < 2 || !mat_input_tokens.contains(data[1])) {
            mars_debug::debug::alert("MARS - Material - Invalid line \"" + line + "\" in file - " + _path);
            continue;
        }

        switch (mat_input_tokens[data[1]]) {
            case MARS_MATERIAL_INPUT_SHADER:
                if (!m_graphics->resources()->load_graphical_resource<shader>(m_graphics->resources()->find_path(data[0], MARS_RESOURCE_TYPE_SHADER, m_graphics->render_type()), m_shader, m_graphics))
                    mars_debug::debug::error("MARS - Failed to load shader - " + m_graphics->resources()->find_path(data[0], MARS_RESOURCE_TYPE_SHADER, m_graphics->render_type()));
                break;
            case MARS_MATERIAL_INPUT_TEXTURE:
                std::shared_ptr<texture> new_texture;
                m_graphics->resources()->load_graphical_resource<texture>(m_graphics->resources()->find_path(data[0], MARS_RESOURCE_TYPE_TEXTURE), new_texture, m_graphics);
                m_textures[data[2]] = new_texture;
                break;
        }
    }

    return true;
}

void material::clean() {

}
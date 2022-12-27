#include <MVRE/graphics/material.hpp>
#include <MVRE/algo/string_helper.hpp>


using namespace mvre_graphics;
using namespace mvre_resources;

std::map<std::string, MVRE_MATERIAL_INPUT> material::mat_input_tokens = {
        { "shader", MVRE_MATERIAL_INPUT_SHADER },
        { "texture", MVRE_MATERIAL_INPUT_TEXTURE }
};

bool material::load_resource(const std::string &_path) {
    std::vector<std::string> lines;
    if (!resource_manager::read_file(_path, lines))
        mvre_debug::debug::error("MVRE - Material - Failed to find material - " + _path);

    for (const auto& line : lines) {
        if (!line.contains(':'))
            continue;

        std::vector<std::string> data = mvre_string::explode(line, ':');

        if (data.size() < 2 || !mat_input_tokens.contains(data[1])) {
            mvre_debug::debug::alert("MVRE - Material - Invalid line \"" + line + "\" in file - " + _path);
            continue;
        }

        switch (mat_input_tokens[data[1]]) {
            case MVRE_MATERIAL_INPUT_SHADER:
                resource_manager::load_graphical_resource<shader>(resource_manager::find_path(data[0], MVRE_RESOURCE_TYPE_SHADER), m_shader, m_instance);
                break;
            case MVRE_MATERIAL_INPUT_TEXTURE:
                texture* new_texture;
                resource_manager::load_graphical_resource<texture>(resource_manager::find_path(data[0], MVRE_RESOURCE_TYPE_TEXTURE), new_texture, m_instance);
                m_textures[data[2]] = new_texture;
                break;
        }
    }

    return true;
}

void material::clean() {

}
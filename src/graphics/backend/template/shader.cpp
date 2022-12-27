#include <MVRE/graphics/backend/template/shader.hpp>
#include <MVRE/algo/string_helper.hpp>

using namespace mvre_graphics;
using namespace mvre_resources;

std::map<std::string, MVRE_SHADER_TOKEN> shader::module_tokens = {
        { "#vertex", MVRE_SHADER_TOKEN_VERTEX },
        { "#fragment", MVRE_SHADER_TOKEN_FRAGMENT },
        { "#attribute", MVRE_SHADER_TOKEN_ATTRIBUTE },
        { "#uniform", MVRE_SHADER_TOKEN_UNIFORM }
};

std::map<std::string, MVRE_UNIFORM_TYPE> shader::uniform_tokens = {
        { "float", MVRE_UNIFORM_TYPE_FLOAT },
        { "matrix", MVRE_UNIFORM_TYPE_MATRIX },
        { "sampler", MVRE_UNIFORM_TYPE_SAMPLER }
};

bool shader::load_shader_file(std::string _path, std::string _path_sufix) {
    std::vector<std::string> lines;
    if (!resource_manager::read_file(_path, lines))
        return false;

    auto reading_mode = MVRE_SHADER_TOKEN_VERTEX;

    for (auto line : lines) {
        if (line.empty())
            continue;

        if (line[0] == '#') {
            auto token = module_tokens.find(line);
            if (token != module_tokens.end()) {
                reading_mode = token->second;
                continue;
            }
        }

        switch (reading_mode) {
            case MVRE_SHADER_TOKEN_VERTEX:
            case MVRE_SHADER_TOKEN_FRAGMENT:
                m_modules[graphics_types::token_to_type(reading_mode)] += line;
                break;
            case MVRE_SHADER_TOKEN_UNIFORM:
                std::vector<std::string> data = mvre_string::explode(line, ' ');

                size_t size = std::strtoul(data[2].c_str(), nullptr, 10);
                if (size == 0)
                    continue;

                size_t binding = std::strtoul(data[3].c_str(), nullptr, 10);

                m_uniforms.push_back(new mvre_shader_uniform(data[0], uniform_tokens[data[1]], size, binding));
                break;
        }
    }

    std::vector<char> mod_data;

    for (auto& module : m_modules) {
        switch (module.first) {
            case MVRE_SHADER_TYPE_VERTEX:
            case MVRE_SHADER_TYPE_FRAGMENT:
                mod_data.resize(0);
                resource_manager::read_binary(resource_manager::resources_locations[MVRE_RESOURCE_TYPE_SHADER] + module.second + _path_sufix, mod_data);
                module.second = std::string(mod_data.begin(), mod_data.end());
                break;
        }
    }

    return true;
}

bool shader::load_resource(const std::string& _path) {
    return load_shader_file(_path);
}

void shader::clean() {
    for (auto& uni : m_uniforms)
        delete uni;
}

mvre_shader_uniform* shader::get_uniform(const std::string& _name) {
    for (auto uni : m_uniforms)
        if (uni->name == _name)
            return uni;

    return nullptr;
}
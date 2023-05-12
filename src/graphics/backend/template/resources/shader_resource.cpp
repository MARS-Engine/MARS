#include <MARS/graphics/backend/template/resources/shader_resource.hpp>
#include <MARS/graphics/backend/template/shader.hpp>
#include <MARS/resources/resource_manager.hpp>
#include <MARS/algo/string_helper.hpp>

using namespace mars_graphics;
using namespace mars_resources;

bool shader_resource::load_resource(const std::string& _path) {
    std::vector<std::string> lines;
    if (!resource_manager::read_file(_path, lines))
        return false;

    auto reading_mode = MARS_SHADER_TOKEN_VERTEX;

    for (auto line : lines) {
        if (line.empty())
            continue;

        if (line[0] == '#') {
            auto token = shader::module_tokens.find(line);
            if (token != shader::module_tokens.end()) {
                reading_mode = token->second;
                continue;
            }
        }

        switch (reading_mode) {
            case MARS_SHADER_TOKEN_VERTEX:
            case MARS_SHADER_TOKEN_FRAGMENT:
                m_data.modules[graphics_types::token_to_type(reading_mode)] += line;
                break;
            case MARS_SHADER_TOKEN_UNIFORM:
                std::vector<std::string> data = mars_string::explode(line, ' ');

                size_t size = std::strtoul(data[2].c_str(), nullptr, 10);
                if (size == 0)
                    continue;

                size_t binding = std::strtoul(data[3].c_str(), nullptr, 10);

                m_data.uniforms.push_back(std::make_shared<mars_shader_uniform>(data[0], shader::uniform_tokens.at(data[1]), size, binding));
                break;
        }
    }

    std::vector<char> mod_data;

    for (auto& module : m_data.modules) {
        switch (module.first) {
            case MARS_SHADER_TYPE_VERTEX:
            case MARS_SHADER_TYPE_FRAGMENT:
                mod_data.resize(0);
            if (!resource_manager::read_binary(resources()->get_location(MARS_RESOURCE_TYPE_SHADER) + m_render_type + "/" + module.second += m_suffix, mod_data))
            return false;
            module.second = std::string(mod_data.begin(), mod_data.end());
            break;
            default:
                mars_debug::debug::alert("MARS - Shader Resource - Invalid Shader Type");
            break;
        }
    }

    return true;
}
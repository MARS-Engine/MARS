#include <MARS/graphics/backend/template/shader.hpp>
#include <MARS/algo/string_helper.hpp>

using namespace mars_graphics;
using namespace mars_resources;

const std::map<std::string, MARS_SHADER_TOKEN> shader::module_tokens = {
        { "#vertex", MARS_SHADER_TOKEN_VERTEX },
        { "#fragment", MARS_SHADER_TOKEN_FRAGMENT },
        { "#attribute", MARS_SHADER_TOKEN_ATTRIBUTE },
        { "#uniform", MARS_SHADER_TOKEN_UNIFORM }
};

const std::map<std::string, MARS_UNIFORM_TYPE> shader::uniform_tokens = {
        { "float", MARS_UNIFORM_TYPE_FLOAT },
        { "matrix", MARS_UNIFORM_TYPE_MATRIX },
        { "buffer", MARS_UNIFORM_TYPE_BUFFER },
        { "sampler", MARS_UNIFORM_TYPE_SAMPLER }
};

std::shared_ptr<mars_shader_uniform> shader::get_uniform(const std::string& _name) {
    for (auto uni : m_data->data().uniforms)
        if (uni->name == _name)
            return uni;

    return nullptr;
}
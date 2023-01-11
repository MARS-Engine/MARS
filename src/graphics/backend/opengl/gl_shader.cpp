#include <MARS/graphics/backend/opengl/gl_shader.hpp>
#include <MARS/executioner/executioner.hpp>

using namespace mars_graphics;
using namespace mars_executioner;

void gl_shader::generate_shader(MARS_SHADER_TYPE _type, const std::string& _data) {
    unsigned int module_id;
    switch (_type) {
        case MARS_SHADER_TYPE_VERTEX:
            module_id = m_shaders_modules[_type] = glCreateShader(GL_VERTEX_SHADER);
            break;
        case MARS_SHADER_TYPE_FRAGMENT:
            module_id = m_shaders_modules[_type] = glCreateShader(GL_FRAGMENT_SHADER);
            break;
    }

    const char* shader_source = _data.data();
    glShaderSource(module_id, 1, &shader_source, nullptr);
    glCompileShader(module_id);

    int  success;
    glGetShaderiv(module_id, GL_COMPILE_STATUS, &success);

    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(module_id, 512, nullptr, infoLog);
        mars_debug::debug::error("MARS - openGL - shader - failed to compile\n " + (std::string)infoLog);
    }

    glAttachShader(id, module_id);

}

bool gl_shader::load_resource(const std::string &_path) {
    if (!load_shader_file(_path))
        return false;

    id = glCreateProgram();

    for (auto &m: m_modules) {
        switch (m.first) {
            case MARS_SHADER_TYPE_VERTEX:
            case MARS_SHADER_TYPE_FRAGMENT:
                generate_shader(m.first, m.second);
                break;
        }
    }

    glLinkProgram(id);

    int success;
    char infoLog[1024];
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(id, 1024, nullptr, infoLog);
        mars_debug::debug::error((std::string)"MARS - OpenGL - Shader - Failed to link " + infoLog);
    }

    glUseProgram(id);

    return true;
}
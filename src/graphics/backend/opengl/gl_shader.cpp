#include <MVRE/graphics/backend/opengl/gl_shader.hpp>
#include <MVRE/executioner/executioner.hpp>

using namespace mvre_graphics;
using namespace mvre_executioner;

void gl_shader::generate_shader(MVRE_SHADER_TYPE _type, const std::string& _data) {
    unsigned int module_id;
    switch (_type) {
        case MVRE_SHADER_TYPE_VERTEX:
            module_id = m_shaders_modules[_type] = glCreateShader(GL_VERTEX_SHADER);
            break;
        case MVRE_SHADER_TYPE_FRAGMENT:
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
        mvre_debug::debug::error("MVRE - openGL - shader - failed to compile\n " + (std::string)infoLog);
    }

    glAttachShader(id, module_id);

}

bool gl_shader::load_resource(const std::string &_path) {
    if (!load_shader_file(_path))
        return false;

    id = glCreateProgram();

    for (auto &m: m_modules) {
        switch (m.first) {
            case MVRE_SHADER_TYPE_VERTEX:
            case MVRE_SHADER_TYPE_FRAGMENT:
                generate_shader(m.first, m.second);
                break;
        }
    }
    glLinkProgram(id);
    glUseProgram(id);

    return true;
}
#include <MVRE/graphics/backend/opengl/gl_shader.hpp>
#include <MVRE/executioner/executioner.hpp>

using namespace mvre_graphics_base;
using namespace mvre_graphics_opengl;
using namespace mvre_executioner;

void gl_shader_uniform::update(void *_data) {
    glUniform4fv(id, 4, (float*)_data);
}

void gl_shader::generate_shader(MVRE_SHADER_TYPE _type, const char* _data) {
    unsigned int module_id;
    switch (_type) {
        case MVRE_SHADER_TYPE_VERTEX:
            module_id = m_shaders_modules[_type] = glCreateShader(GL_VERTEX_SHADER);
            break;
        case MVRE_SHADER_TYPE_FRAGMENT:
            module_id = m_shaders_modules[_type] = glCreateShader(GL_FRAGMENT_SHADER);
            break;
    }

    glShaderSource(module_id, 1, &_data, nullptr);
    glCompileShader(module_id);

    int  success;
    glGetShaderiv(module_id, GL_COMPILE_STATUS, &success);

    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(module_id, 512, nullptr, infoLog);
        mvre_debug::debug::error("gl - shader - failed to compile\n " + (std::string)infoLog);
    }

    glAttachShader(id, module_id);

}

bool gl_shader::load_resource(const std::string &_path) {
    if (!load_shader_file(_path))
        return false;

    executioner_job *job;
    job = new executioner_job([&]() {
        id = glCreateProgram();

        for (auto &m: m_modules) {
            switch (m.first) {
                case MVRE_SHADER_TYPE_VERTEX:
                case MVRE_SHADER_TYPE_FRAGMENT:
                    generate_shader(m.first, m.second.c_str());
                    break;
            }
        }
        glLinkProgram(id);
        glUseProgram(id);

        for (auto& m_uniform : m_uniforms) {

            auto uni = new gl_shader_uniform(*m_uniform);
            delete m_uniform;
            m_uniform = uni;

            uni->id = glGetUniformLocation(id, uni->name.c_str());

            switch (uni->type) {
                case MVRE_UNIFORM_TYPE_FLOAT:
                    switch (uni->size) {
                        case 4:
                            glUniform4f(uni->id, 0.0f, 1.0f, 0.0f, 1.0f);
                            break;
                    }
                    break;
                case MVRE_UNIFORM_TYPE_MATRIX:
                    break;
                case MVRE_UNIFORM_TYPE_SAMPLER:
                    break;
            }
        }
    });

    executioner::add_job(EXECUTIONER_JOB_PRIORITY_IN_FLIGHT, job);
    job->wait();

    return true;
}
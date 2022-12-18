#include <MVRE/graphics/backend/opengl/gl_backend_instance.hpp>
#include <MVRE/graphics/backend/opengl/gl_window.hpp>
#include <glad/glad.h>
#include <MVRE/graphics/backend/opengl/gl_command_buffer.hpp>
#include <MVRE/time/time_helper.hpp>

using namespace mvre_graphics;
using namespace mvre_executioner;

void GLAPIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
        fprintf( stderr, "GL ERROR CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message );
}

void gl_backend_instance::create_with_window(const std::string& _title, mvre_math::vector2<int> _size) {
    raw_window = new gl_window();
    raw_window->initialize(_title, _size);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    raw_window->create();

    m_gl_context = SDL_GL_CreateContext(raw_window->raw_window());

    SDL_GL_MakeCurrent(raw_window->raw_window(), nullptr); //needed for windows -_-

    //prepare opengl render thread
    auto job_high = executioner_job([&]() {
        SDL_GL_MakeCurrent(raw_window->raw_window(), m_gl_context);
        SDL_GL_SetSwapInterval(0);

        //load extensions
        if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
            printf("Failed to load opengl extensions");

        //enable callback
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, nullptr);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    });

    executioner::add_job(mvre_executioner::EXECUTIONER_JOB_PRIORITY_IN_FLIGHT, &job_high);

    //wait to make it's done before executing more opengl
    job_high.wait();

    //create clear job
    clear_job = new executioner_job([&]() {
        glClearColor(0.0, 0.0, 0.0, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    });
    //windows requires this to be on render thread
    swap_job = new executioner_job([&]() { SDL_GL_SwapWindow(raw_window->raw_window()); });

    m_primary_buffer = new gl_command_buffer(this);
}

void gl_backend_instance::update() {
    raw_window->process();
}

void gl_backend_instance::prepare_render() {
    executioner::add_job(EXECUTIONER_JOB_PRIORITY_NORMAL, clear_job);
}

void gl_backend_instance::draw() {
    executioner::add_job(EXECUTIONER_JOB_PRIORITY_IN_FLIGHT, swap_job);
    swap_job->wait();
}

void gl_backend_instance::destroy() {
    delete raw_window;
}
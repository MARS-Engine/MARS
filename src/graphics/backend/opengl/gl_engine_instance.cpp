#include "MVRE/graphics/backend/opengl/gl_engine_instance.hpp"
#include "MVRE/graphics/backend/opengl/gl_context.hpp"

#include <glad/glad.h>

using namespace mvre_graphics_opengl;
using namespace mvre_executioner;

void GLAPIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
  fprintf( stderr, "GL ERROR CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message );
}

void gl_engine_instance::create_with_window(mvre_graphics::window& _window) {

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    _window.create();

    SDL_GL_SetSwapInterval(0);

    //create context
    auto _gl_context = new gl_context(_window);
    _gl_context->create();
    context = _gl_context;
    SDL_GL_MakeCurrent(_window.raw_window, nullptr); //needed for windows

    //prepare opengl render thread
    auto job_high = executioner_job([&]() {
        SDL_GL_MakeCurrent(_window.raw_window, _gl_context->raw_context);

        //load extensions
        if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
            printf("Failed to load opengl extensions");

        //enable callback
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, nullptr);

        //glEnable(GL_DEPTH_TEST);
    });

    executioner::add_job(mvre_executioner::EXECUTIONER_JOB_PRIORITY_IN_FLIGHT, &job_high);

    //wait to make it's done before executing more opengl
    job_high.wait();

    //create clear job
    clear_job = new executioner_job([&]() { glClearColor(0.2f, 0.3f, 0.3f, 1.0f); glClear(GL_COLOR_BUFFER_BIT); });
    //windows requires this to be on render thread
    swap_job = new executioner_job([&]() { SDL_GL_SwapWindow(context->surf_window.raw_window); });
}

void gl_engine_instance::update() {
    executioner::add_job(EXECUTIONER_JOB_PRIORITY_NORMAL, swap_job);
    context->surf_window.process();
}

void gl_engine_instance::clear() {
    executioner::add_job(EXECUTIONER_JOB_PRIORITY_NORMAL, clear_job);
}

void gl_engine_instance::clean() {
    context->clean();
    delete clear_job;
}
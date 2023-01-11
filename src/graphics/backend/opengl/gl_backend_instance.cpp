#include <MARS/graphics/backend/opengl/gl_backend_instance.hpp>
#include <MARS/graphics/backend/opengl/gl_window.hpp>
#include <glad/glad.h>
#include <MARS/graphics/backend/opengl/gl_command_buffer.hpp>
#include <MARS/time/time_helper.hpp>
#include <MARS/graphics/attribute/vertex2.hpp>

using namespace mars_graphics;
using namespace mars_executioner;

void GLAPIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
        fprintf( stderr, "GL ERROR CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message );
    else
        mars_debug::debug::alert((std::string)"MARS - OpenGL - Debug - " + message);
}

void gl_backend_instance::create_with_window(const std::string& _title, const mars_math::vector2<int>& _size) {
    raw_window = new gl_window();
    raw_window->initialize(_title, _size);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    raw_window->create();

    m_gl_context = SDL_GL_CreateContext(raw_window->raw_window());

    SDL_GL_MakeCurrent(raw_window->raw_window(), nullptr); //needed for windows -_-

    m_framebuffer = new gl_framebuffer(this);
    m_framebuffer->add_texture(MARS_TEXTURE_TYPE_F_RGBA16, MARS_TEXTURE_DATA_TYPE_FLOAT);
    m_framebuffer->add_texture(MARS_TEXTURE_TYPE_F_RGBA16, MARS_TEXTURE_DATA_TYPE_FLOAT);
    m_framebuffer->add_texture(MARS_TEXTURE_TYPE_RGBA8, MARS_TEXTURE_DATA_TYPE_UNSIGNED_BYTE);

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

        m_framebuffer->create();
        m_light = new light_manager();
        m_light->load(m_instance);
    });

    executioner::add_job(mars_executioner::EXECUTIONER_JOB_PRIORITY_IN_FLIGHT, &job_high);

    //create clear job
    clear_job = new executioner_job([&]() {
        glClearColor(0.0, 0.0, 0.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_is_deferred_renderer_enabled) {
            m_framebuffer->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
    });

    //windows requires this to be on render thread
    swap_job = new executioner_job([&]() {
        if (m_is_deferred_renderer_enabled) {
            m_framebuffer->unbind();

            m_framebuffer->bind_textures();
            m_light->draw_lights();
        }

        SDL_GL_SwapWindow(raw_window->raw_window());
    });

    m_primary_buffer = new gl_command_buffer(this);

    //wait to make it's done before executing mars opengl
    job_high.wait();

}

void gl_backend_instance::update() {

}

void gl_backend_instance::prepare_render() {
    executioner::add_job(EXECUTIONER_JOB_PRIORITY_NORMAL, clear_job);
}

void gl_backend_instance::draw() {
    executioner::add_job(EXECUTIONER_JOB_PRIORITY_IN_FLIGHT, swap_job);
    swap_job->wait();
    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void gl_backend_instance::destroy() {
    delete m_framebuffer;
    delete raw_window;
}
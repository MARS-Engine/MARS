add_subdirectory(v_backend)
target_sources(MVRE PUBLIC
        v_window.cpp
        v_backend_instance.cpp
        v_shader.cpp
        v_render_pass.cpp
        v_pipeline.cpp
        v_command_buffer.cpp
        v_shader_input.cpp
        v_buffer.cpp)

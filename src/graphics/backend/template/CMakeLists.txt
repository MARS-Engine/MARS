add_subdirectory(builders)
add_subdirectory(resources)
add_subdirectory(stc)

target_sources(MARS PUBLIC
        shader.cpp
        graphics_types.cpp
        texture.cpp
        backend_instance.cpp
        global_buffers.cpp)
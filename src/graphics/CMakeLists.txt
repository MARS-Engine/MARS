add_subdirectory(backend)
add_subdirectory(attribute)
add_subdirectory(renderer)
target_sources(MARS PUBLIC
        material.cpp
        pipeline_manager.cpp
        light_manager.cpp
        graphics_handler.cpp
        graphics_engine.cpp)
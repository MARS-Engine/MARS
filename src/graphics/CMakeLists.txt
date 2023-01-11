add_subdirectory(backend)
add_subdirectory(attribute)
target_sources(MARS PUBLIC
        material.cpp
        pipeline_manager.cpp
        light_manager.cpp)
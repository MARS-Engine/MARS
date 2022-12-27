add_subdirectory(backend)
add_subdirectory(attribute)
target_sources(MVRE PUBLIC
        material.cpp
        pipeline_manager.cpp)
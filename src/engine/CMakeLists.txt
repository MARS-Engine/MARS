add_subdirectory(layers)
target_sources(MARS PUBLIC
        component.cpp
        engine_object.cpp
        engine_handler.cpp
        transform_3d.cpp
        engine_worker.cpp)
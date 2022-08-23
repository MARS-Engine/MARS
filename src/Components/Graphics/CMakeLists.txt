add_subdirectory(2D)
add_subdirectory(3D)
target_sources(MVRE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Camera.cpp)
target_sources(MVRE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Animator.cpp)
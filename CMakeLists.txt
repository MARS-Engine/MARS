cmake_minimum_required(VERSION 3.25)
project(MARS)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")


find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)

add_library(MARS SHARED)

target_compile_options(MARS INTERFACE -msse4.1)

add_subdirectory(third_party/)

target_include_directories(MARS PUBLIC ${SDL2_INCLUDE_DIR})
target_include_directories(MARS PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(MARS PUBLIC ${VULKAN_INCLUDE_DIR})
target_include_directories(MARS PUBLIC include)

target_link_libraries(MARS ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(MARS ${Vulkan_LIBRARIES})
target_link_libraries(MARS ${OPENGL_LIBRARIES})
target_link_libraries(MARS SDL2main SDL2)
target_link_libraries(MARS parallel)

add_subdirectory(src)

add_custom_command(TARGET MARS PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets/ ${CMAKE_BINARY_DIR}/bin/engine/assets)
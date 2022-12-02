cmake_minimum_required(VERSION 3.23)
project(MVRE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)

add_library(MVRE SHARED)

add_subdirectory(third_party/)

target_include_directories(MVRE PUBLIC ${SDL2_INCLUDE_DIR})
target_include_directories(MVRE PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(MVRE PUBLIC ${VULKAN_INCLUDE_DIR})
target_include_directories(MVRE PUBLIC third_party/glad/include/)
target_include_directories(MVRE PUBLIC include)

target_link_libraries(MVRE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(MVRE ${Vulkan_LIBRARIES})
target_link_libraries(MVRE ${OPENGL_LIBRARIES})
target_link_libraries(MVRE SDL2main SDL2)
target_link_libraries(MVRE parallel)

add_subdirectory(src)

#add_custom_command(TARGET MVRE PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets/ ${CMAKE_BINARY_DIR}/bin/engine/assets)
add_library(MVRE SHARED)

add_subdirectory(third_party)
add_subdirectory(src)

target_link_libraries(MVRE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(MVRE ${Vulkan_LIBRARIES})
target_link_libraries(MVRE ${SDL2_LIBRARIES})
target_link_libraries(MVRE vkbootstrap vma tinyobjloader)

target_include_directories(MVRE PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(MVRE PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_include_directories(MVRE PUBLIC ${SDL2_INCLUDE_DIRS})
target_compile_options(MVRE PUBLIC ${FREETYPE_CGLAGS_OTHER})

target_include_directories(MVRE PUBLIC ${CMAKE_SOURCE_DIR}/MVRE/src)
add_subdirectory(imgui)
add_subdirectory(tiny_gltf)
add_subdirectory(stb)
add_subdirectory(fastgltf)

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/win32/targets.cmake")

# fastgltf must be compiled with the same libc++ as the rest of the project
# so that std::filesystem::path (and other STL types) mangle identically.
mars_win32_apply_libcxx(fastgltf)

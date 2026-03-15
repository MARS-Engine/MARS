add_subdirectory(imgui)
add_subdirectory(tiny_gltf)
add_subdirectory(stb)
add_subdirectory(fastgltf)

# fastgltf must be compiled with the same libc++ as the rest of the project
# so that std::filesystem::path (and other STL types) mangle identically.
if(WIN32)
    mars_win32_apply_libcxx(fastgltf)
elseif(LINUX)
    mars_linux_apply_libcxx(fastgltf)
endif()

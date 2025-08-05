find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)


add_library(mars ./src/mars.cpp)
add_subdirectory(src)
add_subdirectory(third_party)

target_include_directories(mars PRIVATE ./include)

# ---- compile phase ----
target_compile_options(mars PRIVATE
        -nostdinc++
        -isystem /home/tech/Documents/code/c++/clang-p2996/build/include/c++/v1
)

target_include_directories(mars PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(mars PRIVATE ${FREETYPE_LIBRARIES})

target_include_directories(mars PRIVATE /usr/include)
target_include_directories(mars PRIVATE ../third_party/glad/include)

# ---- link phase ----
target_link_options(mars PRIVATE
        -stdlib=libc++
        -L/home/tech/Documents/code/c++/clang-p2996/build/lib/x86_64-unknown-linux-gnu
        -Wl,-rpath,/home/tech/Documents/code/c++/clang-p2996/build/lib/x86_64-unknown-linux-gnu
        -L/usr/lib64/
        -L/usr/lib/
        # -L/path/to/your/lib   # add if libc++ isn’t on the default linker search path
)
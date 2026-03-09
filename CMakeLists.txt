include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/win32/dependencies.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/win32/targets.cmake")

mars_require_win32()
mars_win32_prepare_dependencies()

set(MARS_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")

add_library(mars)
target_sources(mars PRIVATE src/mars.cpp)

add_subdirectory(src)
add_subdirectory(third_party)

target_include_directories(
    mars
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${MARS_IMGUI_DIR}"
        "${MARS_IMGUI_DIR}/backends"
)

mars_win32_configure_target(mars)

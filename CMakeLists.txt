find_package(Vulkan REQUIRED)

if(WIN32)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/win32/dependencies.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/win32/targets.cmake")
    mars_win32_prepare_dependencies()
elseif(LINUX)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/linux/dependencies.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/linux/targets.cmake")
    mars_linux_prepare_dependencies()
else()
    message(FATAL_ERROR "mars: Unsupported platform.")
endif()

set(MARS_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")

add_library(mars)
target_sources(mars PRIVATE src/mars.cpp)

option(MARS_FORMATTED_OUTPUT "Enable structured JSON log output (stdout=NDJSON, stderr=human, log.json=full document)" OFF)
if(MARS_FORMATTED_OUTPUT)
    target_compile_definitions(mars PUBLIC MARS_FORMATTED_OUTPUT)
endif()

add_subdirectory(src)
add_subdirectory(third_party)

target_include_directories(
    mars
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${MARS_IMGUI_DIR}"
        "${MARS_IMGUI_DIR}/backends"
)

target_link_libraries(mars PUBLIC
    Vulkan::Vulkan
    fastgltf
)

if(MSVC)
    target_compile_options(mars PRIVATE /arch:AVX2)
else()
    target_compile_options(mars PRIVATE -mavx2)
endif()

if(WIN32)
    mars_win32_configure_target(mars)
elseif(LINUX)
    mars_linux_configure_target(mars)
endif()

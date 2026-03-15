target_sources(
    mars
    PRIVATE
        imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        backends/imgui_impl_sdl3.cpp
        backends/imgui_impl_vulkan.cpp
)

if(WIN32)
    target_sources(mars PRIVATE backends/imgui_impl_dx12.cpp)
endif()

target_include_directories(mars PRIVATE . ./backends)

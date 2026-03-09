include_guard(GLOBAL)

include(FetchContent)

function(mars_win32_prepare_dependencies)
    if(NOT WIN32)
        message(FATAL_ERROR "mars: Win32 dependencies were requested on a non-Windows platform.")
    endif()

    if(NOT TARGET SDL3::SDL3-shared)
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG main
        )
        FetchContent_MakeAvailable(SDL3)
    endif()

    if(NOT TARGET WinPixEventRuntime)
        FetchContent_Declare(
            winpixeventruntime
            URL "https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.240308001"
            DOWNLOAD_NAME "WinPixEventRuntime.1.0.240308001.zip"
        )
        FetchContent_MakeAvailable(winpixeventruntime)

        add_library(WinPixEventRuntime SHARED IMPORTED GLOBAL)
        set_target_properties(
            WinPixEventRuntime
            PROPERTIES
                IMPORTED_LOCATION "${winpixeventruntime_SOURCE_DIR}/bin/x64/WinPixEventRuntime.dll"
                IMPORTED_IMPLIB "${winpixeventruntime_SOURCE_DIR}/bin/x64/WinPixEventRuntime.lib"
                INTERFACE_INCLUDE_DIRECTORIES "${winpixeventruntime_SOURCE_DIR}/Include"
                INTERFACE_COMPILE_DEFINITIONS "USE_PIX"
        )
    endif()
endfunction()

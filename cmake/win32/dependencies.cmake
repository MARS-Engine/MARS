include_guard(GLOBAL)

include(FetchContent)

function(mars_win32_prepare_dependencies)
    if(NOT WIN32)
        message(FATAL_ERROR "mars: Win32 dependencies were requested on a non-Windows platform.")
    endif()

    if(NOT TARGET DXC::dxcompiler)
        FetchContent_Declare(
            DXC
            URL      "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2602/dxc_2026_02_20.zip"
            URL_HASH "SHA256=a1e89031421cf3c1fca6627766ab3020ca4f962ac7e2caa7fab2b33a8436151e"
        )
        FetchContent_MakeAvailable(DXC)

        add_library(DXC::dxcompiler SHARED IMPORTED GLOBAL)
        set_target_properties(DXC::dxcompiler PROPERTIES
            IMPORTED_LOCATION             "${dxc_SOURCE_DIR}/bin/x64/dxcompiler.dll"
            IMPORTED_IMPLIB               "${dxc_SOURCE_DIR}/lib/x64/dxcompiler.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${dxc_SOURCE_DIR}/inc;${dxc_SOURCE_DIR}/inc/Support"
        )
        set(MARS_DXCOMPILER_BIN "${dxc_SOURCE_DIR}/bin/x64/dxcompiler.dll" CACHE PATH "" FORCE)
        set(MARS_DXIL_BIN       "${dxc_SOURCE_DIR}/bin/x64/dxil.dll"       CACHE PATH "" FORCE)
    endif()

    if(CUSTOM_CLANG_PATH)
        find_file(
            MARS_LIBCXX_DLL
            NAMES c++.dll libc++.dll
            PATHS
                "${CUSTOM_CLANG_PATH}/bin"
                "${CUSTOM_CLANG_PATH}/lib"
            NO_DEFAULT_PATH
        )
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
                IMPORTED_IMPLIB   "${winpixeventruntime_SOURCE_DIR}/bin/x64/WinPixEventRuntime.lib"
                INTERFACE_INCLUDE_DIRECTORIES "${winpixeventruntime_SOURCE_DIR}/Include"
                INTERFACE_COMPILE_DEFINITIONS "USE_PIX"
        )
    endif()
endfunction()

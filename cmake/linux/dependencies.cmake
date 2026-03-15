include_guard(GLOBAL)

include(FetchContent)

function(mars_linux_prepare_dependencies)
    if(NOT LINUX)
        message(FATAL_ERROR "mars: Linux dependencies were requested on a non-Linux platform.")
    endif()

    if(NOT TARGET SDL3::SDL3-shared)
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG main
        )
        FetchContent_MakeAvailable(SDL3)
    endif()

    if(NOT TARGET DXC::dxcompiler)
        FetchContent_Declare(
            DXC
            URL      "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2602/linux_dxc_2026_02_20.x86_64.tar.gz"
            URL_HASH "SHA256=a1d3e3b5e1c5685b3eb27d5e8890e41d87df45def05112a2d6f1a63a931f7d60"
        )
        FetchContent_MakeAvailable(DXC)

        add_library(DXC::dxcompiler SHARED IMPORTED GLOBAL)
        set_target_properties(DXC::dxcompiler PROPERTIES
            IMPORTED_LOCATION             "${dxc_SOURCE_DIR}/lib/libdxcompiler.so"
            # include/dxc/ is the root so that #include <dxcapi.h> resolves and
            # relative includes inside dxcapi.h (e.g. "WinAdapter.h") stay valid.
            INTERFACE_INCLUDE_DIRECTORIES "${dxc_SOURCE_DIR}/include/dxc"
        )
    endif()

    find_file(MARS_LIBCXX_DLL    NAMES c++.so libc++.so.1 libc++abi.so.1 PATHS "${CUSTOM_CLANG_PATH}/lib/x86_64-unknown-linux-gnu" NO_DEFAULT_PATH)
endfunction()

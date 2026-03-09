include_guard(GLOBAL)

function(mars_require_win32)
    if(NOT WIN32)
        message(FATAL_ERROR "mars currently supports only Win32 builds.")
    endif()
endfunction()

function(mars_win32_apply_libcxx target)
    target_include_directories("${target}" PRIVATE "${CUSTOM_CLANG_PATH}/include/c++/v1")
    target_compile_options("${target}" PRIVATE -stdlib=libc++)
endfunction()

function(mars_win32_configure_target target)
    mars_require_win32()

    target_include_directories("${target}" PUBLIC "${CUSTOM_CLANG_PATH}/include/c++/v1")

    target_compile_options(
        "${target}"
        PRIVATE
            -freflection-latest
            -fexperimental-library
    )

    target_link_options(
        "${target}"
        PRIVATE
            -stdlib=libc++
            -L"${CUSTOM_CLANG_PATH}/lib"
    )

    target_link_libraries(
        "${target}"
        PUBLIC
            SDL3::SDL3-shared
            WinPixEventRuntime
            "${CUSTOM_CLANG_PATH}/lib/c++.lib"
            d3d12.lib
            dxgi.lib
            user32.lib
            dxcompiler.lib
    )
endfunction()

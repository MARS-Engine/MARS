include_guard(GLOBAL)

function(mars_win32_apply_libcxx target)
    target_include_directories("${target}" PRIVATE "${CUSTOM_CLANG_PATH}/include/c++/v1")
endfunction()

function(mars_win32_configure_target target)
    target_include_directories("${target}" PUBLIC "${CUSTOM_CLANG_PATH}/include/c++/v1")

    target_compile_options(
        "${target}"
        PRIVATE
            -freflection
    )

    target_link_options(
        "${target}"
        PRIVATE
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
            DXC::dxcompiler
            comdlg32.lib
            user32.lib
    )
endfunction()

function(mars_win32_configure target)
    if(MARS_DXCOMPILER_BIN AND MARS_DXIL_BIN)
        add_custom_command(TARGET "${target}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MARS_DXCOMPILER_BIN}" "$<TARGET_FILE_DIR:${target}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MARS_DXIL_BIN}" "$<TARGET_FILE_DIR:${target}>"
        )
    endif()

    if(MARS_LIBCXX_DLL)
        add_custom_command(TARGET "${target}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MARS_LIBCXX_DLL}" "$<TARGET_FILE_DIR:${target}>"
        )
    endif()

    add_custom_command(TARGET "${target}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared> "$<TARGET_FILE_DIR:${target}>"
    )

    add_custom_command(TARGET "${target}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:WinPixEventRuntime> "$<TARGET_FILE_DIR:${target}>"
    )
endfunction()

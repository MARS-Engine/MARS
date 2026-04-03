include_guard(GLOBAL)

function(mars_require_reflection_flags)
    if(NOT DEFINED MARS_REFLECTION_FLAGS OR "${MARS_REFLECTION_FLAGS}" STREQUAL "")
        message(FATAL_ERROR "MARS_REFLECTION_FLAGS must be defined before configuring Mars targets")
    endif()
endfunction()

function(mars_win32_apply_libcxx target)
    if(CUSTOM_CLANG_PATH)
        target_include_directories("${target}" PRIVATE "${CUSTOM_CLANG_PATH}/include/c++/v1")
    endif()
endfunction()

function(mars_win32_configure_target target)
    mars_require_reflection_flags()
    if(CUSTOM_CLANG_PATH)
        target_include_directories("${target}" PUBLIC "${CUSTOM_CLANG_PATH}/include/c++/v1")
    endif()

    target_compile_options(
        "${target}"
        PRIVATE
            ${MARS_REFLECTION_FLAGS}
    )

    if(CUSTOM_CLANG_PATH)
        target_link_options(
            "${target}"
            PRIVATE
                -L"${CUSTOM_CLANG_PATH}/lib"
        )
    endif()

    target_link_libraries(
        "${target}"
        PUBLIC
            SDL3::SDL3-shared
            WinPixEventRuntime
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

include_guard(GLOBAL)

function(mars_require_reflection_flags)
    if(NOT DEFINED MARS_REFLECTION_FLAGS OR "${MARS_REFLECTION_FLAGS}" STREQUAL "")
        message(FATAL_ERROR "MARS_REFLECTION_FLAGS must be defined before configuring Mars targets")
    endif()
endfunction()

function(mars_linux_apply_libcxx target)
    if(CUSTOM_CLANG_PATH)
        target_compile_options("${target}" PRIVATE )
    endif()
endfunction()

function(mars_linux_configure_target target)
    mars_require_reflection_flags()

    if(CUSTOM_CLANG_PATH)
        target_compile_options("${target}" PUBLIC "-nostdinc++")
        target_include_directories("${target}" PUBLIC "${CUSTOM_CLANG_PATH}/include/c++/v1")
        
        target_link_options("${target}" PRIVATE 
            "-L${CUSTOM_CLANG_PATH}/lib"
            "-stdlib=libc++"
        )

        target_link_options("${target}" PUBLIC "-stdlib=libc++")
    endif()

    target_compile_options("${target}" PRIVATE ${MARS_REFLECTION_FLAGS})

    target_link_libraries("${target}" PUBLIC 
        SDL3::SDL3-shared
        DXC::dxcompiler
        pthread
        dl
    )

    if(MARS_DXCOMPILER_BIN AND MARS_DXIL_BIN)
        add_custom_command(TARGET "${target}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MARS_DXCOMPILER_BIN}" "$<TARGET_FILE_DIR:${target}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MARS_DXIL_BIN}" "$<TARGET_FILE_DIR:${target}>"
        )
    elseif(MARS_DXCOMPILER_BIN)
        add_custom_command(TARGET "${target}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MARS_DXCOMPILER_BIN}" "$<TARGET_FILE_DIR:${target}>"
        )
    endif()

    if(MARS_LIBCXX_DLL)
        add_custom_command(TARGET "${target}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MARS_LIBCXX_DLL}" "$<TARGET_FILE_DIR:${target}>"
        )
    endif()

    set(_combined_rpath "$ORIGIN")
    if(CUSTOM_CLANG_PATH)
        list(APPEND _combined_rpath "${CUSTOM_CLANG_PATH}/lib")
        list(APPEND _combined_rpath "${CUSTOM_CLANG_PATH}/lib/x86_64-unknown-linux-gnu")
    endif()

    set_target_properties("${target}" PROPERTIES 
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "${_combined_rpath}"
        LINK_FLAGS "-Wl,--disable-new-dtags"
    )
endfunction()

function(mars_linux_configure target)

    add_custom_command(TARGET "${target}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared> "$<TARGET_FILE_DIR:${target}>"
    )
        
    add_custom_command(TARGET "${target}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared> "$<TARGET_FILE_DIR:${target}>/libSDL3.so.0"
    )

    add_custom_command(TARGET "${target}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:DXC::dxcompiler> "$<TARGET_FILE_DIR:${target}>"
    )

    # Also copy the whole DXC lib folder in case the real SONAME filename is
    # a versioned file (e.g. libdxcompiler.so.1) or there are additional
    # runtime dependencies shipped alongside the main library.
    if(DEFINED dxc_SOURCE_DIR)
        add_custom_command(TARGET "${target}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${dxc_SOURCE_DIR}/lib"
                "$<TARGET_FILE_DIR:${target}>"
        )
    endif()

        if(CUSTOM_CLANG_PATH)
        set(_mars_rpath "$ORIGIN;${CUSTOM_CLANG_PATH}/lib;${CUSTOM_CLANG_PATH}/lib/x86_64-unknown-linux-gnu")
    else()
        set(_mars_rpath "$ORIGIN")
    endif()

    set_target_properties("${target}"  PROPERTIES
        BUILD_RPATH "${_mars_rpath}"                        # look in executable dir and toolchain lib
        INSTALL_RPATH "${_mars_rpath}"                      # installed rpath
        BUILD_WITH_INSTALL_RPATH TRUE
        LINK_FLAGS "-Wl,--disable-new-dtags" 
    )
    # linker rpath is provided via the BUILD_RPATH/INSTALL_RPATH properties above
endfunction()
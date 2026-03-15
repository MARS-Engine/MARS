include_guard(GLOBAL)

function(mars_linux_apply_libcxx target)
    if(CUSTOM_CLANG_PATH)
        target_compile_options("${target}" PRIVATE -stdlib=libc++)
    endif()
endfunction()

function(mars_linux_configure_target target)
    if(CUSTOM_CLANG_PATH)
        # -stdlib=libc++ makes Clang automatically add its own libc++ include
        # paths — no need to add them manually (doing so causes double-inclusion
        # and cxxabi.h conflicts).
        target_compile_options("${target}" PUBLIC -stdlib=libc++)
        target_link_options("${target}" PRIVATE
            -stdlib=libc++
            -L"${CUSTOM_CLANG_PATH}/lib"
        )
    endif()

    target_compile_options(
        "${target}"
        PRIVATE
            -freflection-latest
            -fexperimental-library
    )

    target_link_libraries("${target}" PUBLIC SDL3::SDL3-shared DXC::dxcompiler)
endfunction()

function(mars_linux_configure_parcel target)
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
        set(_parcel_rpath "\$ORIGIN:${CUSTOM_CLANG_PATH}/lib:${CUSTOM_CLANG_PATH}/lib/x86_64-unknown-linux-gnu")
    else()
        set(_parcel_rpath "\$ORIGIN")
    endif()

    set_target_properties("${target}"  PROPERTIES
        BUILD_RPATH "${_parcel_rpath}"                        # look in executable dir and toolchain lib
        INSTALL_RPATH "${_parcel_rpath}"                      # installed rpath
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    # linker rpath is provided via the BUILD_RPATH/INSTALL_RPATH properties above
endfunction()

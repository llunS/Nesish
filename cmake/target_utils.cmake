function(configure_cxx i_tgt i_cxx_std)
    set_target_properties(${i_tgt} PROPERTIES
        CXX_STANDARD ${i_cxx_std}
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS 0
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN hidden
    )
endfunction()

function(configure_c i_tgt i_c_std)
    set_target_properties(${i_tgt} PROPERTIES
        C_STANDARD ${i_c_std}
        C_STANDARD_REQUIRED TRUE
        C_EXTENSIONS 0
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN hidden
    )
endfunction()

function(configure_warnings i_tgt)
    if(MSVC)
        target_compile_options(${i_tgt} PRIVATE
            /W4 # warning level 4 is enough
            /WX # warnings as errors
            /analyze # enable code analysis warnings
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # lots of warnings and all warnings as errors
        target_compile_options(${i_tgt} PRIVATE
            -Wall -Wextra -pedantic -Werror
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${i_tgt} PRIVATE
            -Wall -Wextra -pedantic -Werror
            -Wno-error=type-limits
        )
    else()
        message(FATAL_ERROR "Platform not supported yet!")
    endif()
endfunction()

function(configure_vc_options i_tgt)
    if(MSVC)
        target_compile_options(${i_tgt} PRIVATE
            ${ARGN}
        )
    endif()
endfunction()

function(configure_optimizations i_tgt)
    # May report Clang compiler, so check first
    if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        # no-op
    elseif(MSVC)
        target_compile_options(${i_tgt} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:/O2>
            $<$<CONFIG:Release,RelWithDebInfo>:/GL>
        )
        target_link_options(${i_tgt} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:/LTCG>
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${i_tgt} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:-O3>
            $<$<CONFIG:Release,RelWithDebInfo>:-flto=full>
        )
        target_link_options(${i_tgt} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:-O3>
            $<$<CONFIG:Release,RelWithDebInfo>:-flto=full>
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${i_tgt} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:-O3>
            $<$<CONFIG:Release,RelWithDebInfo>:-flto=auto>
        )
        target_link_options(${i_tgt} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:-O3>
            $<$<CONFIG:Release,RelWithDebInfo>:-flto=auto>
        )
    else()
        message(FATAL_ERROR "Platform not supported yet!")
    endif()
endfunction()

function(configure_em_options i_tgt)
    if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        # Optimizations
        target_compile_options(${i_tgt} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:-O3>
            $<$<CONFIG:Release,RelWithDebInfo>:-flto=full>
        )
        target_link_options(${i_tgt} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:-O3>
            $<$<CONFIG:Release,RelWithDebInfo>:-flto=full>
        )

        set(em_flags
            "-fno-rtti"
            "-fno-exceptions"
            "-sDISABLE_EXCEPTION_THROWING=1"
            "-sDISABLE_EXCEPTION_CATCHING=1"
            "-mnontrapping-fptoint"
        )
        set(em_cxxflags
        )
        set(em_ldflags
            "-sWASM=1"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sEXIT_RUNTIME=0"
            "-sENVIRONMENT=web"
            # Uncomment this to track throw source
            # "-Wl,--trace-symbol=__cxa_throw"
        )
        target_compile_options(${i_tgt} PRIVATE ${em_cxxflags} ${em_flags})
        target_link_options(${i_tgt} PRIVATE ${em_ldflags} ${em_flags})
    endif()
endfunction()

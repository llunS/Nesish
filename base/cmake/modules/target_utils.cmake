function(add_cxx i_tgt i_cxx_std)
    set_target_properties(${i_tgt} PROPERTIES
        CXX_STANDARD ${i_cxx_std}
        CXX_STANDARD_REQUIRED TRUE
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN hidden
    )
endfunction()

function(add_warnings i_tgt)
    if(MSVC)
        target_compile_options(${i_tgt} PRIVATE
            /W4 # warning level 4 is enough
            /WX # warnings as errors
            /analyze # enable code analysis warnings
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # lots of warnings and all warnings as errors
        target_compile_options(${i_tgt} PRIVATE
            -Wall -Wextra -pedantic -Werror
        )
    else()
        # @TODO
    endif()
endfunction()

function(add_vc_compile_opts i_tgt)
    if(MSVC)
        target_compile_options(${i_tgt} PRIVATE
            ${ARGN}
        )
    endif()
endfunction()

function(add_opts i_tgt)
    if(MSVC)
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
            $<$<CONFIG:Release,RelWithDebInfo>:-flto=full>
        )
    else()
        # @TODO
    endif()
endfunction()

function(xform_shader i_shader_relpath i_out_reldir i_include_guard_prefix i_ns)
    set(shader_src_path ${CMAKE_CURRENT_SOURCE_DIR}/${i_shader_relpath})

    get_filename_component(shader_name ${shader_src_path} NAME)
    string(REPLACE "." "_"
       shader_name
       ${shader_name}
    )

    # shader_name_c
    set(shader_name_c ${shader_name})
    # shader_src_c
    file(STRINGS ${shader_src_path} shader_src_c NEWLINE_CONSUME)
    string(REPLACE "\\" ""
        shader_src_c
        ${shader_src_c}
    )
    # include_guard_c
    string(TOUPPER ${shader_name} shader_name_upper)
    set(include_guard_c "${i_include_guard_prefix}_SHADER_${shader_name_upper}")
    # namespace_c
    set(namespace_c ${i_ns})

    # Configure
    configure_file(
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/shader.hpp.in
        ${CMAKE_CURRENT_SOURCE_DIR}/${i_out_reldir}/${shader_name}.hpp
        @ONLY
    )

    # Trigger a CMake rerun if shader source is changed.
    include(rerun_watch)
    rerun_watch(${shader_src_path})
endfunction()

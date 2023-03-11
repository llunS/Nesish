function(gen_dllexport_header i_tgt i_dst_dir)
    if(ARGC GREATER 2)
        set(i_prefix_name ${ARGV2})
    endif()

    if(i_prefix_name)
        string(TOUPPER ${i_prefix_name} prefix_name_upper)
    else()
        string(TOUPPER ${i_tgt} prefix_name_upper)
    endif()

    include(GenerateExportHeader)
    set(header_file_name dllexport)
    GENERATE_EXPORT_HEADER(${i_tgt}
        EXPORT_FILE_NAME ${header_file_name}.h
        PREFIX_NAME ${prefix_name_upper}_
        EXPORT_MACRO_NAME API
    )

    # Remove first to ensure a latest copy.
    file(REMOVE ${i_dst_dir}/${header_file_name}.h)
    file(COPY
        ${CMAKE_CURRENT_BINARY_DIR}/${header_file_name}.h
        DESTINATION ${i_dst_dir}
    )
endfunction()

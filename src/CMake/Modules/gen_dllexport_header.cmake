function(gen_dllexport_header i_module)
    if(ARGC GREATER 1)
        set(i_macro_name ${ARGV1})
    endif()

    if(i_macro_name)
        string(TOUPPER ${i_macro_name} macro_name_upper)
    else()
        string(TOUPPER ${i_module} macro_name_upper)
    endif()

    include(GenerateExportHeader)
    set(header_file_name dllexport)
    GENERATE_EXPORT_HEADER(${i_module}
        EXPORT_FILE_NAME ${header_file_name}.h
        PREFIX_NAME LN_
        EXPORT_MACRO_NAME ${macro_name_upper}_API
    )

    set(dst_dir ${CMAKE_CURRENT_SOURCE_DIR})
    # Remove first to ensure a latest copy.
    file(REMOVE ${dst_dir}/${header_file_name}.h)
    file(COPY
        ${CMAKE_CURRENT_BINARY_DIR}/${header_file_name}.h
        DESTINATION ${dst_dir}
    )
endfunction()

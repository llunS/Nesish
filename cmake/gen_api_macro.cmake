function(gen_api_macro i_target i_basename o_dir)
    # Generate in binary dir
    include(GenerateExportHeader)
    set(fileName api)
    GENERATE_EXPORT_HEADER(${i_target}
        EXPORT_FILE_NAME ${fileName}.h
        EXPORT_MACRO_NAME ${i_basename}_API
    )

    # Copy from binary to source, delete first
    file(REMOVE ${o_dir}/${fileName}.h)
    file(COPY
        ${CMAKE_CURRENT_BINARY_DIR}/${fileName}.h
        DESTINATION ${o_dir}
    )
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${fileName}.h)
endfunction()

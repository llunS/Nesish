# include a test
function(inc_test i_directory i_test)

    # Define target
    function(def_test i_directory i_test)
        string(REPLACE "/" "_" snake_dir ${i_directory})
        set(tgt_name ${snake_dir}_${i_test})
        add_executable(${tgt_name} ${i_directory}/${i_test}.cpp)
        include(target_utils)
        configure_cxx(${tgt_name} 11)
        configure_warnings(${tgt_name})
        configure_vc_options(${tgt_name} /wd6285 /wd6326)
        configure_optimizations(${tgt_name})

        find_package(GTest CONFIG REQUIRED)
        target_link_libraries(${tgt_name} PRIVATE
            Nesish
            NesishBase
            fmt::fmt-header-only
            GTest::gtest_main
        )

        # Add GTest test cases to CTest.
        include(GoogleTest)
        gtest_discover_tests(${tgt_name})
    endfunction()

    def_test(${i_directory} ${i_test})

    # Copy test files
    function(copy_test_file i_directory i_test i_test_filepath)
        string(REPLACE "/" "_" snake_dir ${i_directory})
        set(tgt_name ${snake_dir}_${i_test})

        get_filename_component(test_file_basename ${i_test_filepath} NAME)

        add_custom_command(TARGET ${tgt_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${i_test_filepath} $<TARGET_FILE_DIR:${tgt_name}>/${test_file_basename}
        )
    endfunction()

    foreach(test_file ${ARGN})
        copy_test_file(${i_directory} ${i_test} ${i_directory}/${test_file})
    endforeach()

endfunction()

# CPU
inc_test(cpu/nestest test nestest.nes nestest.log)

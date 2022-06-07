function(add_test i_directory i_test)
    set(target_name ${i_directory}_${i_test})
    add_executable(${target_name} ${i_directory}/${i_test}.cpp)

    find_package(GTest CONFIG REQUIRED)
    target_link_libraries(${target_name} PRIVATE
        Console
        GTest::gtest_main
    )

    if(MSVC)
        # Disable C6326 warning.
        target_compile_options(${target_name} PRIVATE /wd6326)
    endif()

    # Add GTest test cases to CTest.
    include(GoogleTest)
    gtest_discover_tests(${target_name})
endfunction()

function(copy_test_file i_directory i_test i_test_filepath)
    set(target_name ${i_directory}_${i_test})

    get_filename_component(test_file_basename ${i_test_filepath} NAME)

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${i_test_filepath} $<TARGET_FILE_DIR:${target_name}>/${test_file_basename}
    )
endfunction()


add_test(common path_test)
add_test(common filesystem_test)

add_test(nestest cpu_test)
copy_test_file(nestest cpu_test nestest/nestest.nes)
copy_test_file(nestest cpu_test nestest/nestest.log)

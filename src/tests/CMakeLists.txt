function(add_test i_directory i_test)
    set(exe_name ${i_directory}_${i_test})
    add_executable(${exe_name} ${i_directory}/${i_test}.cpp)

    find_package(GTest CONFIG REQUIRED)
    target_link_libraries(${exe_name} PRIVATE
        Console
        GTest::gtest_main
    )

    if(MSVC)
        # Disable C6326 warning.
        target_compile_options(${exe_name} PRIVATE /wd6326)
    endif()

    # Add GTest test cases to CTest.
    include(GoogleTest)
    gtest_discover_tests(${exe_name})
endfunction()

function(copy_test_file i_test_file)
    file(COPY
        ${CMAKE_CURRENT_SOURCE_DIR}/${i_test_file}
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()


add_test(common path_test)
add_test(common filesystem_test)

add_test(nestest cpu_test)
copy_test_file(nestest/nestest.nes)
copy_test_file(nestest/nestest.log)

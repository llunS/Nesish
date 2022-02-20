function(AddTest i_directory i_test)
    set(exe_name ${i_directory}_${i_test})
    add_executable(${exe_name} ${i_directory}/${i_test}.cpp)

    find_package(GTest CONFIG REQUIRED)
    target_link_libraries(${exe_name} PRIVATE Common GTest::gtest_main)

    if(MSVC)
        # Disable C6326 warning.
        target_compile_options(${exe_name} PRIVATE /wd6326)
    endif()

    # Add GTest test cases to CTest.
    include(GoogleTest)
    gtest_discover_tests(${exe_name})
endfunction()

AddTest(common path_test)
AddTest(common filesystem_test)

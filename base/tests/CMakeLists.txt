# Define a test
function(inc_test i_test)
    set(tgt_name ${i_test})
    add_executable(${tgt_name} ${i_test}.cpp)

    find_package(GTest CONFIG REQUIRED)
    target_link_libraries(${tgt_name} PRIVATE
        NesishBase
        GTest::gtest_main
    )

    # Add GTest test cases to CTest.
    include(GoogleTest)
    gtest_discover_tests(${tgt_name})
endfunction()

if (MSVC)
    add_compile_options(
        # suppress some warnings
        /wd6326
    )
endif()

inc_test(path_test)
inc_test(filesystem_test)

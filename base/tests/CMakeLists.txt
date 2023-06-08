# Define a test
function(inc_test i_test)
    set(tgt_name ${i_test})
    add_executable(${tgt_name} ${i_test}.cpp)
    include(target_utils)
    configure_cxx(${tgt_name} 11)
    configure_warnings(${tgt_name})
    configure_vc_options(${tgt_name} /wd6326)
    configure_optimizations(${tgt_name})

    find_package(GTest CONFIG REQUIRED)
    target_link_libraries(${tgt_name} PRIVATE
        NesishBase
        GTest::gtest_main
    )

    # Add GTest test cases to CTest.
    include(GoogleTest)
    gtest_discover_tests(${tgt_name})
endfunction()

inc_test(path_test)
inc_test(filesystem_test)

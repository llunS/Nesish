set(ModuleName Common)
add_library(${ModuleName})


# --------------------------------------------
# Include directories
# --------------------------------------------

target_include_directories(${ModuleName} PUBLIC ../)


# --------------------------------------------
# Source files
# --------------------------------------------

list(APPEND Sources logger.cpp)
list(APPEND Sources "path.cpp")
if(LT_TARGET_MACOS)
    list(APPEND Sources path_macos.mm)
endif()
list(APPEND Sources filesystem.cpp)

target_sources(${ModuleName} PRIVATE ${Sources})


# --------------------------------------------
# Dependencies
# --------------------------------------------

find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${ModuleName} PUBLIC spdlog::spdlog_header_only)


# --------------------------------------------
# Tests
# --------------------------------------------

if(LN_BUILD_TESTS)
    function(AddTest i_test)
        add_executable(${i_test} tests/${i_test}.cpp)

        find_package(GTest CONFIG REQUIRED)
        target_link_libraries(${i_test} PRIVATE ${ModuleName} GTest::gtest_main)

        if(MSVC)
            # Disable C6326 warning.
            target_compile_options(${i_test} PRIVATE /wd6326)
        endif()

        # Add GTest test cases to CTest.
        include(GoogleTest)
        gtest_discover_tests(${i_test})
    endfunction()

    AddTest(path_test)
    AddTest(filesystem_test)
endif()

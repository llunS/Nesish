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
list(APPEND Sources filesystem.cpp)
if(LT_TARGET_MACOS)
    list(APPEND Sources path_macos.mm)
    list(APPEND Sources filesystem_macos.cpp)
endif()

target_sources(${ModuleName} PRIVATE ${Sources})


# --------------------------------------------
# Dependencies
# --------------------------------------------

find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${ModuleName} PUBLIC spdlog::spdlog_header_only)

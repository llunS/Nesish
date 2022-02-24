set(module_name Common)
add_library(${module_name} SHARED)

include(gen_dllexport_header)
gen_dllexport_header(${module_name})


# --------------------------------------------
# Include directories
# --------------------------------------------

target_include_directories(${module_name} PUBLIC ../)


# --------------------------------------------
# Source files
# --------------------------------------------

list(APPEND sources logger.cpp)
list(APPEND sources "path.cpp")
list(APPEND sources filesystem.cpp)
if(LT_TARGET_MACOS)
    list(APPEND sources path_macos.mm)
    list(APPEND sources filesystem_macos.cpp)
endif()

target_sources(${module_name} PRIVATE ${sources})


# --------------------------------------------
# Dependencies
# --------------------------------------------

find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${module_name} PUBLIC spdlog::spdlog_header_only)

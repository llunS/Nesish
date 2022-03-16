set(module_name SFMLApp)
add_library(${module_name} SHARED)

include(gen_dllexport_header)
gen_dllexport_header(${module_name})


# --------------------------------------------
# Include directories
# --------------------------------------------

target_include_directories(${module_name} PUBLIC ../../)


# --------------------------------------------
# Source files
# --------------------------------------------

list(APPEND sources sf_app.cpp)
list(APPEND sources sf_controller.cpp)

target_sources(${module_name} PRIVATE ${sources})


# --------------------------------------------
# Dependencies
# --------------------------------------------

find_package(SFML COMPONENTS window CONFIG REQUIRED)

target_link_libraries(${module_name} PRIVATE
    sfml-window
    Console
)

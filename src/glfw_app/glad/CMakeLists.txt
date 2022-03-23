set(module_name glad)
add_library(${module_name} SHARED)


# --------------------------------------------
# Include directories
# --------------------------------------------

target_include_directories(${module_name} PUBLIC ./)


# --------------------------------------------
# Source files
# --------------------------------------------

target_sources(${module_name} PRIVATE glad.c)

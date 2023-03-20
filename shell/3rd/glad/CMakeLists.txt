set(module_name glad)
add_library(${module_name} SHARED)

# -- Source files

target_sources(${module_name} PRIVATE glad.c)

# -- Definitions

target_compile_definitions(${module_name} PUBLIC -DGLAD_GLAPI_EXPORT PRIVATE -DGLAD_GLAPI_EXPORT_BUILD)

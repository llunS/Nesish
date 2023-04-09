set(tgt_name glad)
add_library(${tgt_name} SHARED)
include(target_utils)
add_opts(${tgt_name})

# -- Source files

target_sources(${tgt_name} PRIVATE glad.c)

# -- Include directories

target_include_directories(${tgt_name} PUBLIC ./)

# -- Definitions

target_compile_definitions(${tgt_name} PUBLIC -DGLAD_GLAPI_EXPORT PRIVATE -DGLAD_GLAPI_EXPORT_BUILD)

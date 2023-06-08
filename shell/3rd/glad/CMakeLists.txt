set(tgt_name glad)
add_library(${tgt_name} STATIC)
include(target_utils)
configure_optimizations(${tgt_name})

# -- Source files

target_sources(${tgt_name} PRIVATE glad.c)

# -- Include directories

target_include_directories(${tgt_name} PUBLIC ./)

set(tgt_name blip_buf)
add_library(${tgt_name} STATIC)
include(target_utils)
configure_optimizations(${tgt_name})

# -- Source files

target_sources(${tgt_name} PRIVATE blip_buf.c)

# -- Include directories

target_include_directories(${tgt_name} PUBLIC ./)

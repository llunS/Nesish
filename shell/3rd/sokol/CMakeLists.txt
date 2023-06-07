set(tgt_name sokol)
add_library(${tgt_name} INTERFACE)

# -- Include directories

target_include_directories(${tgt_name} INTERFACE .)

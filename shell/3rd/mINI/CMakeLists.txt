set(tgt_name mINI_header_only)
add_library(${tgt_name} INTERFACE)

# -- Include directories

target_include_directories(${tgt_name} INTERFACE src/)

set(tgt_name ImGuiFileBrowser)
add_library(${tgt_name} STATIC)
include(target_utils)
configure_cxx(${tgt_name} 11)
configure_optimizations(${tgt_name})

# -- Source files

target_sources(${tgt_name} PRIVATE ImGuiFileBrowser.cpp)

# -- Include directories

target_include_directories(${tgt_name} PUBLIC ./)

# -- Dependencies

# imgui found in parent directory
target_link_libraries(${tgt_name} PUBLIC imgui)

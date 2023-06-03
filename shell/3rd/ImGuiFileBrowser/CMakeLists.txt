set(tgt_name ImGuiFileBrowser)
add_library(${tgt_name} STATIC)
include(target_utils)
add_cxx(${tgt_name} 11)
add_opts(${tgt_name})

# -- Source files

target_sources(${tgt_name} PRIVATE ImGuiFileBrowser.cpp)

# -- Include directories

target_include_directories(${tgt_name} PUBLIC ./)

# -- Dependencies

target_link_libraries(${tgt_name} PUBLIC imgui::imgui)

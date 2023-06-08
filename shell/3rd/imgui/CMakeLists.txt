set(tgt_name imgui)
add_library(${tgt_name} STATIC)
include(target_utils)
configure_cxx(${tgt_name} 11)
configure_optimizations(${tgt_name})

# -- Source files

target_sources(${tgt_name} PRIVATE
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp

    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
)

# -- Include directories

target_include_directories(${tgt_name} PUBLIC ./)
target_include_directories(${tgt_name} PUBLIC backends/)

# -- Dependencies

# glfw found in parent directory
target_link_libraries(${tgt_name} PRIVATE glfw)

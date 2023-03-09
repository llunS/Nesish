set(module_name GLFWApp)
add_library(${module_name} SHARED)

include(gen_dllexport_header)
gen_dllexport_header(${module_name} App)


# --------------------------------------------
# Include directories
# --------------------------------------------

target_include_directories(${module_name} PUBLIC ../../)


# --------------------------------------------
# Source files
# --------------------------------------------

list(APPEND sources app.cpp)
list(APPEND sources controller.cpp)

list(APPEND sources rendering/error.cpp)
list(APPEND sources rendering/shader.cpp)
list(APPEND sources rendering/renderer.cpp)
list(APPEND sources rendering/texture.cpp)

list(APPEND sources gui/platform_window.cpp)
list(APPEND sources gui/emulator_window.cpp)
list(APPEND sources gui/debugger_window.cpp)
list(APPEND sources gui/rect_cut.cpp)

list(APPEND sources blip_buf/blip_buf.c)
# 3rd-Party code
if (MSVC)
    set_source_files_properties(blip_buf/blip_buf.c PROPERTIES COMPILE_OPTIONS "/WX-")
endif()
list(APPEND sources audio/resampler.cpp)
list(APPEND sources audio/pcm_writer.cpp)

target_sources(${module_name} PRIVATE ${sources})


# --------------------------------------------
# Dependencies
# --------------------------------------------

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(RtAudio CONFIG REQUIRED)

add_subdirectory(glad)

target_link_libraries(${module_name} PRIVATE
    glfw
    imgui::imgui
    glad
    RtAudio::rtaudio
    Console
)

target_compile_definitions(${module_name} PUBLIC -DGLFW_INCLUDE_NONE)


# --------------------------------------------
# Transform shaders
# --------------------------------------------

include(xform_shader)
xform_shader(resources/shader_in/screen_rect.vert resources/shader sh)
xform_shader(resources/shader_in/screen_rect.frag resources/shader sh)

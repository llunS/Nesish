cmake_minimum_required(VERSION 3.25.1)
project(NesishShell)


# -- Global compiler/linker options/flags

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Put binary outputs to single directory
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

# Add cmake scripts
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)


# -- Target

set(tgt_name NesishShell)
add_executable(${tgt_name})
set_target_properties(${tgt_name} PROPERTIES OUTPUT_NAME Nesish)
include(target_utils)
add_cxx(${tgt_name} 11)
add_warnings(${tgt_name})
add_vc_compile_opts(${tgt_name} /wd6285)
add_opts(${tgt_name})

# --- Include directories

target_include_directories(${tgt_name} PRIVATE src 3rd)

# --- Source files

set(sources "")

list(APPEND sources src/app.cpp)
list(APPEND sources src/controller.cpp)

list(APPEND sources src/rendering/error.cpp)
list(APPEND sources src/rendering/shader.cpp)
list(APPEND sources src/rendering/renderer.cpp)
list(APPEND sources src/rendering/texture.cpp)

list(APPEND sources src/gui/platform_window.cpp)
list(APPEND sources src/gui/emulator_window.cpp)
list(APPEND sources src/gui/debugger_window.cpp)
list(APPEND sources src/gui/rect_cut.cpp)

list(APPEND sources src/logger.cpp)

list(APPEND sources src/audio/resampler.cpp)
list(APPEND sources src/audio/pcm_writer.cpp)

list(APPEND sources src/main.cpp)

target_sources(${tgt_name} PRIVATE ${sources})

# --- Dependencies

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(RtAudio CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core core)
# @TODO: Further separation of projects
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../base base)
add_subdirectory(3rd/glad)
add_subdirectory(3rd/blip_buf)

target_link_libraries(${tgt_name} PRIVATE
    glfw
    imgui::imgui
    glad
    blip_buf
    RtAudio::rtaudio
    spdlog::spdlog_header_only
    Nesish
    NesishBase
)

target_compile_definitions(${tgt_name} PRIVATE -DGLFW_INCLUDE_NONE)

# --- Transform shaders

include(xform_shader)
xform_shader(resources/shader/screen_rect.vert src/shaders sh)
xform_shader(resources/shader/screen_rect.frag src/shaders sh)
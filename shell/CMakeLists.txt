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
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake)

# -- CMake options

# Because it supports Emscripten
option(SH_USE_SOKOL_AUDIO "Use sokol_audio instead of RtAudio" ON)

# -- Target

set(tgt_name NesishShell)
add_executable(${tgt_name})
set_target_properties(${tgt_name} PROPERTIES OUTPUT_NAME Nesish)
include(target_utils)
configure_cxx(${tgt_name} 11)
configure_warnings(${tgt_name})
configure_vc_options(${tgt_name} /wd6285)
configure_optimizations(${tgt_name})

# --- Include directories

target_include_directories(${tgt_name} PRIVATE src)

# --- Source files

set(sources "")

list(APPEND sources src/gui/application.cpp)
list(APPEND sources src/gui/messager.cpp)
list(APPEND sources src/gui/ppu_debugger.cpp)
list(APPEND sources src/gui/custom_key.cpp)

list(APPEND sources src/rendering/error.cpp)
list(APPEND sources src/rendering/shader.cpp)
list(APPEND sources src/rendering/renderer.cpp)
list(APPEND sources src/rendering/texture.cpp)

list(APPEND sources src/audio/resampler.cpp)
list(APPEND sources src/audio/pcm_writer.cpp)
if(SH_USE_SOKOL_AUDIO)
    list(APPEND sources src/audio/backend_sokol.cpp)
    list(APPEND sources src/audio/sokol_log.cpp)
else()
    list(APPEND sources src/audio/backend_rtaudio.cpp)
endif()

list(APPEND sources src/input/controller.cpp)

list(APPEND sources src/misc/logger.cpp)
list(APPEND sources src/misc/config.cpp)

list(APPEND sources src/main.cpp)

target_sources(${tgt_name} PRIVATE ${sources})

# --- Dependencies

find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core core)
# @TODO: Formal separation of CMake projects using its export and import functionality
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../base base)
add_subdirectory(3rd/glad)
add_subdirectory(3rd/blip_buf)
add_subdirectory(3rd/imgui) # before ImGuiFileBrowser
add_subdirectory(3rd/ImGuiFileBrowser)
add_subdirectory(3rd/mINI)

target_link_libraries(${tgt_name} PRIVATE
    glfw
    imgui
    spdlog::spdlog_header_only
    glad
    blip_buf
    ImGuiFileBrowser
    mINI_header_only
    Nesish
    NesishBase
)
if(SH_USE_SOKOL_AUDIO)
    add_subdirectory(3rd/sokol)
    target_link_libraries(${tgt_name} PRIVATE sokol)
    target_compile_definitions(${tgt_name} PRIVATE -DSH_USE_SOKOL_AUDIO)
else()
    find_package(RtAudio CONFIG REQUIRED)
    target_link_libraries(${tgt_name} PRIVATE RtAudio::rtaudio)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(${tgt_name} PRIVATE -DSH_TGT_MACOS)

    if(SH_USE_SOKOL_AUDIO)
        find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
        target_link_libraries(${tgt_name} PRIVATE ${AUDIOTOOLBOX_LIBRARY})
    endif()
endif()
target_compile_definitions(${tgt_name} PRIVATE -DGLFW_INCLUDE_NONE)

# --- Transform shaders to compile-time data

include(xform_shader)
xform_shader(resources/shader/screen_rect.vert src/shaders sh)
xform_shader(resources/shader/screen_rect.frag src/shaders sh)

# --- Create all sub folders for config files
# Since using fopen or fstream won’t handle sub-directories

# If a directory already exists it will be silently ignored.
add_custom_command(
    TARGET ${tgt_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:${tgt_name}>/config
)

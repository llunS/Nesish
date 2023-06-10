cmake_minimum_required(VERSION 3.25.1)
project(NesishShell)

# -- Target platform

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(SH_TGT_WEB ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SH_TGT_MACOS ON)
endif()

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
if(NOT SH_TGT_WEB)
    set_target_properties(${tgt_name} PROPERTIES OUTPUT_NAME Nesish)
else()
    set_target_properties(${tgt_name} PROPERTIES OUTPUT_NAME index)
endif()
include(target_utils)
configure_cxx(${tgt_name} 11)
configure_warnings(${tgt_name})
configure_vc_options(${tgt_name} /wd6285)
if(SH_TGT_WEB)
    configure_em_options(${tgt_name})
else()
    configure_optimizations(${tgt_name})
endif()

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
if(NOT SH_TGT_WEB)
    list(APPEND sources src/audio/pcm_writer.cpp)
endif()
if(SH_USE_SOKOL_AUDIO OR SH_TGT_WEB)
    list(APPEND sources src/audio/backend_sokol.cpp)
    list(APPEND sources src/audio/sokol_log.cpp)
else()
    list(APPEND sources src/audio/backend_rtaudio.cpp)
endif()

list(APPEND sources src/input/controller.cpp)

list(APPEND sources src/misc/logger.cpp)
list(APPEND sources src/misc/config.cpp)
if(SH_TGT_WEB)
    list(APPEND sources src/misc/web_utils.cpp)
endif()

list(APPEND sources src/main.cpp)

target_sources(${tgt_name} PRIVATE ${sources})

# --- Dependencies

if(NOT SH_TGT_WEB)
    find_package(glfw3 CONFIG REQUIRED)
endif()

if(SH_TGT_WEB)
    set(FMT_NO_EXCEPTIONS ON CACHE BOOL "-fno-exceptions on Web" FORCE)
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core core)
# @TODO: Formal separation of CMake projects using its export and import functionality
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../base base)

if(SH_TGT_WEB)
    set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "-fno-exceptions on Web" FORCE)
endif()
add_subdirectory(3rd/spdlog)
add_subdirectory(3rd/blip_buf)
add_subdirectory(3rd/imgui) # before ImGuiFileBrowser
add_subdirectory(3rd/mINI)
if(NOT SH_TGT_WEB)
    add_subdirectory(3rd/glad)
    add_subdirectory(3rd/ImGuiFileBrowser)
endif()

target_link_libraries(${tgt_name} PRIVATE
    imgui
    spdlog::spdlog_header_only
    blip_buf
    mINI_header_only
    Nesish
    NesishBase
)

if(NOT SH_TGT_WEB)
    target_link_libraries(${tgt_name} PRIVATE glfw)
    target_link_libraries(${tgt_name} PRIVATE glad)
    target_link_libraries(${tgt_name} PRIVATE ImGuiFileBrowser)
else()
    # Enable EM_ASM_XXX blocks
    target_compile_options(${tgt_name} PRIVATE "-Wno-dollar-in-identifier-extension")

    target_link_options(${tgt_name} PRIVATE
        "-lidbfs.js"
        "-sFILESYSTEM=1"
        "-sUSE_GLFW=3"
        # https://emscripten.org/docs/optimizing/Optimizing-WebGL.html#which-gl-mode-to-target
        # https://emscripten.org/docs/optimizing/Optimizing-WebGL.html#migrating-to-webgl-2
        # The link suggests that 2.0 has improvements over 1.0, try it without profiling
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
        "-sASYNCIFY=1"
        "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/template/shell_minimal.html"
    )

    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

if(SH_USE_SOKOL_AUDIO OR SH_TGT_WEB)
    add_subdirectory(3rd/sokol)
    target_link_libraries(${tgt_name} PRIVATE sokol)
    target_compile_definitions(${tgt_name} PRIVATE -DSH_USE_SOKOL_AUDIO)
else()
    find_package(RtAudio CONFIG REQUIRED)
    target_link_libraries(${tgt_name} PRIVATE RtAudio::rtaudio)
endif()

if(SH_TGT_MACOS)
    target_compile_definitions(${tgt_name} PRIVATE -DSH_TGT_MACOS)

    if(SH_USE_SOKOL_AUDIO)
        find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
        target_link_libraries(${tgt_name} PRIVATE ${AUDIOTOOLBOX_LIBRARY})
    endif()
elseif(SH_TGT_WEB)
    target_compile_definitions(${tgt_name} PRIVATE -DSH_TGT_WEB)
endif()

target_compile_definitions(${tgt_name} PRIVATE -DGLFW_INCLUDE_NONE)

# --- Transform shaders to compile-time data

if(NOT SH_TGT_WEB)
    include(xform_shader)
    xform_shader(resources/shader/opengl33/screen_rect.vert src/shaders sh)
    xform_shader(resources/shader/opengl33/screen_rect.frag src/shaders sh)
endif()

# --- Create all sub folders for config files
# Since using fopen or fstream won’t handle sub-directories
if(NOT SH_TGT_WEB)
    # If a directory already exists it will be silently ignored.
    add_custom_command(
        TARGET ${tgt_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${tgt_name}>/config # value synced with the one in code
    )
endif()

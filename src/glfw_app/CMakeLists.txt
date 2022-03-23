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

target_sources(${module_name} PRIVATE ${sources})


# --------------------------------------------
# Dependencies
# --------------------------------------------

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_subdirectory(glad)

target_link_libraries(${module_name} PRIVATE
    glfw
    imgui::imgui
    glad
    Console
)

target_compile_definitions(${module_name} PUBLIC -DGLFW_INCLUDE_NONE)


# --------------------------------------------
# Transform shaders
# --------------------------------------------

include(xform_shader)
xform_shader(resources/shader_in/triangle.vert resources/shader LN_GLFWAPP ln_app)
xform_shader(resources/shader_in/triangle.frag resources/shader LN_GLFWAPP ln_app)

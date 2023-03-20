set(module_name blip_buf)
add_library(${module_name} STATIC)

# -- Source files

target_sources(${module_name} PRIVATE blip_buf.c)

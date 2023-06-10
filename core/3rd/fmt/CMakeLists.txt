option(FMT_NO_EXCEPTIONS "Compile with -fno-exceptions" OFF)

list(FIND CMAKE_CXX_COMPILE_FEATURES "cxx_variadic_templates" index)
if (${index} GREATER -1)
  # Use cxx_variadic_templates instead of more appropriate cxx_std_11 for
  # compatibility with older CMake versions.
  set(FMT_REQUIRED_FEATURES cxx_variadic_templates)
endif ()
message(STATUS "Required features: ${FMT_REQUIRED_FEATURES}")

add_library(fmt-header-only INTERFACE)
add_library(fmt::fmt-header-only ALIAS fmt-header-only)

target_compile_definitions(fmt-header-only INTERFACE FMT_HEADER_ONLY=1)
target_compile_features(fmt-header-only INTERFACE ${FMT_REQUIRED_FEATURES})

target_include_directories(fmt-header-only INTERFACE include)

if(FMT_NO_EXCEPTIONS)
  target_compile_definitions(fmt-header-only INTERFACE FMT_EXCEPTIONS=0)
endif()

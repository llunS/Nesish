add_library(lua515 STATIC)
set_property(TARGET lua515 PROPERTY C_STANDARD 90)
include(target_utils)
add_opts(lua515)

target_sources(lua515 PRIVATE
    # core
    src/lapi.c
    src/lcode.c
    src/ldebug.c
    src/ldo.c
    src/ldump.c
    src/lfunc.c
    src/lgc.c
    src/llex.c
    src/lmem.c
	src/lobject.c
    src/lopcodes.c
    src/lparser.c
    src/lstate.c
    src/lstring.c
    src/ltable.c
    src/ltm.c
    src/lundump.c
    src/lvm.c
    src/lzio.c
    # lib
    src/lauxlib.c
    src/lbaselib.c
    src/ldblib.c
    src/liolib.c
    src/lmathlib.c
    src/loslib.c
    src/ltablib.c
    src/lstrlib.c
    src/loadlib.c
    src/linit.c
)

target_include_directories(lua515 PUBLIC include)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(lua515 PRIVATE
        -DLUA_USE_LINUX
        # Too much deprecated warnings
        # -DLUA_USE_MACOSX
    )
elseif(WIN32)
    # OK
else()
    message(FATAL_ERROR "Platform not supported yet!")
endif()

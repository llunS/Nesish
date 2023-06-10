#pragma once

#ifdef SH_TGT_WEB

// Target ES2/WebGL1, the minimal
#include <GLES2/gl2.h>

// Vertex arrays are not supported on ES2/WebGL1 unless Emscripten uses
// extensions
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>
#define glBindVertexArray glBindVertexArrayOES
#define glGenVertexArrays glGenVertexArraysOES
#define glDeleteVertexArrays glDeleteVertexArraysOES

#else

#include "glad.h"

#endif

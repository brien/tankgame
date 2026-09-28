#pragma once

// Keep API/header differences at the context/backend boundary. The shared
// programmable backend itself uses only the OpenGL 2.1 / GLES2 common subset.
#ifdef __EMSCRIPTEN__
#include <GLES2/gl2.h>
#elif defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#endif

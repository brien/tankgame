#pragma once

// Keep API/header differences at the context/backend boundary. Modern desktop
// entry points are never taken from these headers; GLFunctions loads them from
// the current SDL context. This matters on Windows, whose system header stops
// at OpenGL 1.1.
#ifdef __EMSCRIPTEN__
#include <GLES2/gl2.h>
#elif defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

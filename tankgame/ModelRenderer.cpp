#ifdef _WIN32
// If building in windows:
#pragma warning(disable : 4996)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
//if not:
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include "ModelRenderer.h"
#include "math.h"


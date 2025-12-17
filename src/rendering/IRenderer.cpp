#include "IRenderer.h"
#include "../Logger.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

void IRenderer::CheckGLError(const char* operation) const
{
#ifdef DEBUG
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        const char* errorStr;
        
        switch (error) {
            case GL_INVALID_ENUM:
                errorStr = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                errorStr = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                errorStr = "GL_INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                errorStr = "GL_OUT_OF_MEMORY";
                break;
            default:
                Logger::Get().Write("OpenGL Error in %s: Unknown error code: %d\n", operation, error);
                return;
        }
        Logger::Get().Write("OpenGL Error in %s: %s\n", operation, errorStr);
    }
#else
    // In release builds, do nothing to avoid performance impact
    (void)operation; // Suppress unused parameter warning
#endif
}

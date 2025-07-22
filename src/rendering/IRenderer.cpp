#include "IRenderer.h"
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
        std::cerr << "OpenGL Error in " << operation << ": ";
        
        switch (error) {
            case GL_INVALID_ENUM:
                std::cerr << "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                std::cerr << "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                std::cerr << "GL_INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                std::cerr << "GL_OUT_OF_MEMORY";
                break;
            default:
                std::cerr << "Unknown error code: " << error;
                break;
        }
        std::cerr << std::endl;
    }
#else
    // In release builds, do nothing to avoid performance impact
    (void)operation; // Suppress unused parameter warning
#endif
}

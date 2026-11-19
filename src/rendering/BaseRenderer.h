#ifndef BASERENDERER_H
#define BASERENDERER_H

#include "IRenderer.h"

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

/**
 * Base implementation class for common renderer functionality
 * 
 * Provides default implementations and common utilities that most
 * renderers will need, reducing code duplication across renderer classes.
 * 
 * Features:
 * - Common OpenGL state management
 * - Initialization state tracking
 * - Helper methods for common operations
 * - Default render state setup/cleanup
 */
class BaseRenderer : public IRenderer
{
public:
    BaseRenderer() = default;
    virtual ~BaseRenderer() = default;
    
    /**
     * Default initialization - marks renderer as initialized
     * Derived classes should override and call this base implementation
     */
    bool Initialize() override;
    
    /**
     * Default cleanup - marks renderer as uninitialized
     * Derived classes should override and call this base implementation
     */
    void Cleanup() override;
    
    /**
     * Default render state setup - enables common OpenGL settings
     * Sets up depth testing, face culling, and blending
     */
    void SetupRenderState() override;
    
    /**
     * Default render state cleanup - restores OpenGL defaults
     * Passes establish and restore their own explicit state
     */
    void CleanupRenderState() override;
    
protected:
    /**
     * Check if renderer is ready for rendering operations
     * @return true if renderer has been initialized successfully
     */
    bool IsReady() const { return isInitialized; }
    
    /**
     * Common OpenGL setup for 3D rendering
     * Enables depth testing and face culling
     */
    void Setup3DRenderState();
    
};

#endif // BASERENDERER_H

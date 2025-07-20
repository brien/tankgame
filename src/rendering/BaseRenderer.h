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
     * Sets up depth testing, face culling, and smooth shading
     */
    void SetupRenderState() override;
    
    /**
     * Default render state cleanup - restores OpenGL defaults
     * Restores previous OpenGL state settings
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
     * Enables depth testing, face culling, and sets up lighting
     */
    void Setup3DRenderState();
    
    /**
     * Common OpenGL setup for 2D rendering (UI, HUD)
     * Disables depth testing, enables blending for transparency
     */
    void Setup2DRenderState();
    
    /**
     * Helper to enable/disable OpenGL lighting
     * @param enable true to enable lighting, false to disable
     */
    void SetLighting(bool enable);
    
    /**
     * Helper to set up alpha blending for transparency effects
     * @param enable true to enable blending, false to disable
     */
    void SetBlending(bool enable);
    
    /**
     * Helper to save current OpenGL matrix
     */
    void PushMatrix();
    
    /**
     * Helper to restore previously saved OpenGL matrix
     */
    void PopMatrix();
    
private:
    // Store previous OpenGL state for restoration
    struct RenderState {
        GLboolean depthTestEnabled;
        GLboolean lightingEnabled;
        GLboolean blendEnabled;
        GLboolean cullFaceEnabled;
        GLint matrixMode;
    } previousState;
    
    bool stateStored = false;
    
    /**
     * Store current OpenGL state for later restoration
     */
    void StoreRenderState();
    
    /**
     * Restore previously stored OpenGL state
     */
    void RestoreRenderState();
};

#endif // BASERENDERER_H

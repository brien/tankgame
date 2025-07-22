#ifndef IRENDERER_H
#define IRENDERER_H

/**
 * Base interface for all rendering components in the tankgame engine
 * 
 * This interface establishes a common contract for all renderer classes,
 * ensuring consistent lifecycle management and rendering state handling.
 * 
 * Design Principles:
 * - Separation of concerns: Renderers only handle OpenGL operations
 * - Consistent lifecycle: Initialize -> Render -> Cleanup pattern
 * - State management: Setup and cleanup render state properly
 * - Resource management: Clear ownership of OpenGL resources
 */
class IRenderer
{
public:
    virtual ~IRenderer() = default;
    
    /**
     * Initialize renderer resources and OpenGL state
     * Called once during application startup or when renderer is created
     * 
     * @return true if initialization successful, false otherwise
     */
    virtual bool Initialize() = 0;
    
    /**
     * Clean up renderer resources and OpenGL state
     * Called during application shutdown or when renderer is destroyed
     * Should release all OpenGL resources (textures, buffers, display lists, etc.)
     */
    virtual void Cleanup() = 0;
    
    /**
     * Setup OpenGL render state required for this renderer
     * Called before rendering operations
     * Should set up blending, depth testing, lighting, etc. as needed
     */
    virtual void SetupRenderState() = 0;
    
    /**
     * Restore previous OpenGL render state
     * Called after rendering operations
     * Should restore OpenGL state to what it was before SetupRenderState()
     */
    virtual void CleanupRenderState() = 0;
    
protected:
    /**
     * Check if renderer has been properly initialized
     * Subclasses can use this to validate state before rendering
     */
    bool isInitialized = false;
    
    /**
     * Helper method to check for OpenGL errors during development
     * Can be called after OpenGL operations to catch issues early
     */
    void CheckGLError(const char* operation) const;
    
private:
    // Prevent copying of renderers (they manage OpenGL resources)
    IRenderer(const IRenderer&) = delete;
    IRenderer& operator=(const IRenderer&) = delete;
    
protected:
    // Allow move operations for derived classes
    IRenderer() = default;
    IRenderer(IRenderer&&) = default;
    IRenderer& operator=(IRenderer&&) = default;
};

#endif // IRENDERER_H

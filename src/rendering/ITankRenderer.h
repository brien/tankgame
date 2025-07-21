#pragma once

#include "RenderData.h"
#include <vector>

/**
 * Interface for tank rendering components.
 * 
 * This interface defines the contract for all tank renderer implementations,
 * allowing for different rendering strategies for different tank types
 * (player tanks, enemy tanks, special tanks, etc.).
 * 
 * Design Principles:
 * - Data-driven rendering: Uses TankRenderData to avoid coupling with game logic
 * - Flexible implementation: Allows different rendering strategies per tank type
 * - Clean separation: Renderers only handle OpenGL operations, not game state
 * - Consistent interface: All tank renderers follow the same contract
 */
class ITankRenderer {
public:
    virtual ~ITankRenderer() = default;
    
    /**
     * Initialize the tank renderer.
     * Sets up any required OpenGL resources, display lists, textures, etc.
     * 
     * @return true if initialization successful, false otherwise
     */
    virtual bool Initialize() = 0;
    
    /**
     * Clean up the tank renderer.
     * Releases any OpenGL resources allocated during initialization.
     */
    virtual void Cleanup() = 0;
    
    /**
     * Renders a single tank using the provided render data.
     * 
     * @param data TankRenderData containing all necessary rendering information
     *             including position, rotation, colors, health, effects, etc.
     */
    virtual void Render(const TankRenderData& data) = 0;
    
    /**
     * Renders multiple tanks using the provided render data.
     * Default implementation calls Render() for each tank individually,
     * but can be overridden for batch rendering optimizations.
     * 
     * @param tanks Vector of TankRenderData for all tanks to render
     */
    virtual void RenderMultiple(const std::vector<TankRenderData>& tanks) {
        for (const auto& tank : tanks) {
            if (tank.alive) {
                Render(tank);
            }
        }
    }
    
    /**
     * Set up OpenGL render state for tank rendering.
     * Called before rendering tanks, allows renderer to configure
     * lighting, blending, depth testing, etc.
     */
    virtual void SetupRenderState() = 0;
    
    /**
     * Clean up OpenGL render state after tank rendering.
     * Called after rendering tanks, should restore OpenGL state
     * or clean up any temporary state changes.
     */
    virtual void CleanupRenderState() = 0;
};

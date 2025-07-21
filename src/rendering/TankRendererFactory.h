#pragma once

#include "ITankRenderer.h"
#include <memory>

/**
 * Factory for creating tank renderer instances.
 * 
 * This factory encapsulates the logic for creating appropriate tank renderers
 * based on tank type, allowing for easy extension and configuration of different
 * rendering strategies without tight coupling to specific renderer classes.
 * 
 * Design Principles:
 * - Factory pattern: Centralizes object creation logic
 * - Extensibility: Easy to add new tank renderer types
 * - Polymorphism: Returns ITankRenderer interface for consistent usage
 * - Memory safety: Uses smart pointers for automatic resource management
 */
class TankRendererFactory {
public:
    /**
     * Tank renderer types supported by the factory.
     */
    enum class RendererType {
        PLAYER_TANK,    // Advanced renderer for player tanks with effects and UI
        ENEMY_TANK,     // Basic renderer for enemy tanks
        UNIFIED         // Unified renderer that handles both player and enemy tanks
    };
    
    /**
     * Creates a tank renderer of the specified type.
     * 
     * @param type The type of tank renderer to create
     * @return Smart pointer to the created tank renderer
     */
    static std::unique_ptr<ITankRenderer> CreateRenderer(RendererType type);
    
    /**
     * Convenience method for creating a renderer based on player status.
     * 
     * @param isPlayer True for player tank renderer, false for enemy tank renderer
     * @return Smart pointer to the created tank renderer
     */
    static std::unique_ptr<ITankRenderer> CreateRenderer(bool isPlayer);
    
    /**
     * Creates the default unified tank renderer.
     * This renderer can handle both player and enemy tanks efficiently.
     * 
     * @return Smart pointer to the unified tank renderer
     */
    static std::unique_ptr<ITankRenderer> CreateUnifiedRenderer();
    
private:
    // Private constructor to prevent instantiation
    TankRendererFactory() = default;
};

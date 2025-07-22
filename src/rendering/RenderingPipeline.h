#pragma once

#include "IRenderer.h"
#include "RenderData.h"
#include "ViewportManager.h"
#include "CameraManager.h"
#include "ResourceManager.h"
#include "TerrainRenderer.h"
#include "BulletRenderer.h"
#include "EffectRenderer.h"
#include "ItemRenderer.h"
#include "ITankRenderer.h"
#include "TankRendererFactory.h"
#include "HUDRenderer.h"
#include "MenuRenderer.h"
#include <memory>

/**
 * Centralized rendering pipeline that orchestrates all rendering operations.
 * 
 * This class serves as the main coordinator for the rendering system,
 * managing the rendering order, viewport setup, camera positioning,
 * and delegating to specialized renderers for different object types.
 * 
 * Design Principles:
 * - Single responsibility: Orchestrates rendering, doesn't handle individual objects
 * - Dependency injection: Receives managers and resources as constructor parameters
 * - Consistent interface: Always renders complete scenes in proper order
 * - Performance oriented: Minimizes state changes and optimizes rendering order
 * - Extensible: Easy to add new rendering stages or object types
 */
class RenderingPipeline : public IRenderer {
public:
    /**
     * Constructor with injected dependencies.
     * 
     * @param viewport ViewportManager for handling split-screen layout
     * @param camera CameraManager for camera positioning and behavior
     * @param resources ResourceManager for accessing display lists, textures, etc.
     */
    RenderingPipeline(ViewportManager& viewport, CameraManager& camera, 
                     ResourceManager& resources);
    
    virtual ~RenderingPipeline() = default;
    
    // IRenderer interface implementation
    bool Initialize() override;
    void Cleanup() override;
    void SetupRenderState() override;
    void CleanupRenderState() override;
    
    /**
     * Renders a complete scene for the specified player.
     * Handles viewport setup, camera positioning, and rendering all objects.
     * 
     * @param scene Complete scene data containing all objects to render
     * @param playerIndex Index of the player whose view to render (0 or 1)
     */
    void RenderScene(const SceneData& scene, int playerIndex);
    
    /**
     * Renders scenes for all players (split-screen support).
     * 
     * @param scene Complete scene data containing all objects to render
     */
    void RenderAllPlayerViews(const SceneData& scene);
    
    /**
     * Get the current rendering statistics.
     * Useful for performance monitoring and debugging.
     */
    struct RenderStats {
        int tanksRendered;
        int bulletsRendered;
        int effectsRendered;
        int itemsRendered;
        float renderTime;
    };
    
    const RenderStats& GetRenderStats() const { return renderStats; }
    
    /**
     * Configure viewports for the given number of players.
     * Handles single-player and split-screen setup.
     * 
     * @param numPlayers Number of active players (1 or 2)
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     */
    void ConfigureViewports(int numPlayers, int screenWidth, int screenHeight);
    
private:
    // Injected dependencies
    ViewportManager& viewportManager;
    CameraManager& cameraManager;
    ResourceManager& resourceManager;
    
    // Specialized renderers (delegates for different object types)
    TerrainRenderer terrainRenderer;
    BulletRenderer bulletRenderer;
    EffectRenderer effectRenderer;
    ItemRenderer itemRenderer;
    std::unique_ptr<ITankRenderer> tankRenderer;
    
    // UI renderers
    HUDRenderer hudRenderer;
    MenuRenderer menuRenderer;
    
    // Rendering statistics
    mutable RenderStats renderStats;
    
    // Main rendering stages
    void SetupSceneForPlayer(const SceneData& scene, int playerIndex);
    void RenderSkybox(const SceneData& scene);
    void RenderTerrain(const TerrainRenderData& terrain);
    void RenderGameObjects(const SceneData& scene);
    void RenderTransparentObjects(const SceneData& scene);
    void RenderUIElements(const SceneData& scene, int playerIndex);
    
    // Object-specific rendering methods
    void RenderTanks(const std::vector<TankRenderData>& tanks);
    void RenderBullets(const std::vector<BulletRenderData>& bullets);
    void RenderEffects(const std::vector<EffectRenderData>& effects);
    void RenderItems(const std::vector<ItemRenderData>& items);
    
    // Rendering utilities
    void ClearBuffers();
    void SetupLighting(const SceneData& scene);
    void OptimizeRenderOrder(std::vector<TankRenderData>& tanks, const Vector3& cameraPos);
    void UpdateRenderStats(const SceneData& scene);
    
    // State management
    void PushRenderState();
    void PopRenderState();
    
    // Performance optimization
    bool ShouldRenderObject(const Vector3& position, const Vector3& cameraPos, float maxDistance = 100.0f);
    void PerformFrustumCulling(const SceneData& scene, int playerIndex);
};

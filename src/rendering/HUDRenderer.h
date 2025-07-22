#ifndef HUDRENDERER_H
#define HUDRENDERER_H

#include "BaseRenderer.h"
#include "HUDData.h"

// Forward declaration
struct HUDRenderData;
struct MenuRenderData;
struct DebugRenderData;

/**
 * HUDRenderer - Specialized renderer for HUD (Heads-Up Display) elements
 * 
 * This class handles rendering of all player HUD elements including:
 * - Health and energy bars
 * - Targeting lines
 * - Combo and special ability meters
 * - FPS counter and debug information
 * - Menus and UI overlays
 * 
 * The renderer uses data-driven rendering, taking HUDRenderData structures
 * that contain all necessary information without any game logic dependencies.
 */
class HUDRenderer : public BaseRenderer {
public:
    HUDRenderer();
    virtual ~HUDRenderer() = default;

    // BaseRenderer interface implementation
    virtual bool Initialize() override;
    virtual void Cleanup() override;

    /**
     * Render HUD for a single player
     * @param hudData Complete HUD data for one player
     */
    void RenderPlayerHUD(const HUDRenderData& hudData);
    
    /**
     * Render HUDs for all players
     * @param playerHUDs Vector of HUD data for all players
     */
    void RenderAllPlayerHUDs(const std::vector<HUDRenderData>& playerHUDs);
    
    /**
     * Render complete UI including HUDs, menus, and debug info
     * @param uiData Complete UI rendering data
     */
    void RenderCompleteUI(const UIRenderData& uiData);
    
    /**
     * Render menu system
     * @param menuData Menu rendering data
     */
    void RenderMenu(const MenuRenderData& menuData);
    
    /**
     * Render debug information
     * @param debugData Debug rendering data
     */
    void RenderDebugInfo(const DebugRenderData& debugData);

private:
    // Individual HUD element rendering methods
    void RenderTargetingLine(const HUDRenderData& hudData);
    void RenderHealthBar(const HUDRenderData& hudData);
    void RenderEnergyBar(const HUDRenderData& hudData);
    void RenderChargeBar(const HUDRenderData& hudData);
    void RenderComboMeter(const HUDRenderData& hudData);
    void RenderSpecialMeter(const HUDRenderData& hudData);
    void RenderFPSCounter(const HUDRenderData& hudData);
    
    // Menu rendering methods
    void RenderMenuBackground(const MenuRenderData& menuData);
    void RenderMenuOptions(const MenuRenderData& menuData);
    void RenderMenuOption(const std::string& text, float x, float y, 
                         bool selected, const Vector3& color);
    
    // Utility rendering methods
    void SetupHUDProjection();
    void RestoreGameProjection();
    void RenderHUDText(const char* text, float x, float y, const Vector3& color);
    void RenderProgressBar(float x, float y, float width, float height,
                          float progress, const Vector3& color, 
                          const Vector3& backgroundColor);
    void RenderQuad(float x, float y, float width, float height, const Vector3& color);
    void RenderTexturedQuad(float x, float y, float width, float height, 
                           unsigned int textureId, const Vector3& color);
    
    // State management
    void SetupHUDRenderState();
    void CleanupHUDRenderState();
    void SetupTextRenderState();
    void CleanupTextRenderState();
    
    // Helper methods
    void ApplyColor(const Vector3& color, float alpha = 1.0f);
    void SetupStippledLine(float animationTime);
    void CleanupStippledLine();
    
    // Texture management (for HUD icons)
    bool LoadHUDTextures();
    void CleanupHUDTextures();
    
    // HUD texture IDs
    enum HUDTextures {
        TEXTURE_HEALTH_ICON = 0,
        TEXTURE_ENERGY_ICON,
        TEXTURE_COMBO_NUMBERS,
        TEXTURE_COUNT
    };
    
    unsigned int hudTextures[TEXTURE_COUNT];
    bool texturesLoaded;
};

#endif // HUDRENDERER_H

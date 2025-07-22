#ifndef MENURENDERER_H
#define MENURENDERER_H

#include "BaseRenderer.h"
#include "HUDData.h"

/**
 * MenuRenderer - Specialized renderer for game menus
 * 
 * This class handles rendering of the game's menu system, including:
 * - Main menu
 * - Pause menu
 * - Settings menu
 * - Menu option highlighting and selection
 * 
 * Based on the original GraphicsTask::DrawMenu implementation but with
 * data-driven rendering using MenuRenderData structures.
 */
class MenuRenderer : public BaseRenderer {
public:
    MenuRenderer();
    virtual ~MenuRenderer() = default;

    // BaseRenderer interface implementation
    virtual bool Initialize() override;
    virtual void Cleanup() override;

    /**
     * Render complete menu system
     * @param menuData Menu rendering data
     */
    void RenderMenu(const MenuRenderData& menuData);

private:
    // Menu rendering methods
    void SetupMenuProjection();
    void RestoreGameProjection();
    void SetupMenuRenderState();
    void CleanupMenuRenderState();
    
    // Individual menu element rendering
    void RenderMenuBackground(const MenuRenderData& menuData);
    void RenderMenuOption(int optionIndex, const MenuRenderData& menuData);
    void RenderSelectedOption(int optionIndex, const MenuRenderData& menuData);
    void RenderNormalOption(int optionIndex, const MenuRenderData& menuData);
    
    // Utility methods
    void GetOptionPosition(int optionIndex, float& x, float& y, float& width, float& height);
    void ApplyColor(const Vector3& color, float alpha = 1.0f);
    void RenderOptionQuad(float x, float y, float width, float height, 
                         const Vector3& color, float alpha, bool filled = true);
    
    // Menu layout constants
    static constexpr float MENU_OPTION_WIDTH = 0.32f;     // 0.35f - 0.03f from original
    static constexpr float MENU_OPTION_HEIGHT = 0.09f;    // 0.15f - 0.06f from original
    static constexpr float MENU_OPTION_SPACING = 0.10f;   // Vertical spacing between options
    static constexpr float MENU_START_X = -0.52f;         // 0.03f - 0.55f from original
    static constexpr float MENU_START_Y = -0.19f;         // -0.06f - 0.13f from original
    
    // Menu colors (from original code)
    static constexpr float SELECTED_LINE_COLOR[3] = {0.0f, 1.0f, 1.0f}; // Cyan
    static constexpr float SELECTED_FILL_COLOR[4] = {0.0f, 0.4f, 0.4f, 0.1f}; // Semi-transparent cyan
};

#endif // MENURENDERER_H

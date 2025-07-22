#pragma once

#include "BaseRenderer.h"
#include "RenderData.h"
#include <vector>

/**
 * Handles rendering of all game items (power-ups).
 * Follows the new rendering architecture by accepting render data instead of game objects.
 */
class ItemRenderer : public BaseRenderer {
public:
    ItemRenderer();
    virtual ~ItemRenderer() = default;
    
    /**
     * Renders all items using the provided render data.
     * @param items Vector of ItemRenderData containing all necessary rendering information
     */
    void RenderItems(const std::vector<ItemRenderData>& items);
    
    /**
     * Renders a single item using the provided render data.
     * @param item ItemRenderData containing rendering information for one item
     */
    void RenderItem(const ItemRenderData& item);
    
protected:
    void SetupRenderState() override;
    void CleanupRenderState() override;
    
private:
    /**
     * Sets the color based on the item type (tank type determines power-up color).
     * @param itemType The type of item/power-up
     */
    void SetItemColor(TankType itemType);
};

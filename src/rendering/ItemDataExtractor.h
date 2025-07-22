#pragma once

#include "RenderData.h"
#include "../Item.h"
#include <vector>

/**
 * Extracts rendering data from Item objects for the ItemRenderer.
 * This class serves as a bridge between game logic (Item) and rendering system.
 */
class ItemDataExtractor {
public:
    /**
     * Extracts rendering data from a single Item object.
     * @param item The Item object to extract data from
     * @return ItemRenderData structure containing all necessary rendering information
     */
    static ItemRenderData ExtractRenderData(const Item& item);
    
    /**
     * Extracts rendering data from a collection of Item objects.
     * Only extracts data from alive items.
     * @param items Vector of Item objects to process
     * @return Vector of ItemRenderData structures for rendering
     */
    static std::vector<ItemRenderData> ExtractRenderData(const std::vector<Item>& items);
};

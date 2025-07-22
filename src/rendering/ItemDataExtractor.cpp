#include "ItemDataExtractor.h"

ItemRenderData ItemDataExtractor::ExtractRenderData(const Item& item) {
    ItemRenderData data;
    
    // Extract position
    data.position = Vector3(item.x, item.y, item.z);
    
    // Extract item type for mesh selection
    data.itemType = item.type;
    
    // Extract rotation for spinning animation
    data.rotationY = item.ry;
    
    // Set visibility based on alive status
    data.visible = item.alive;
    
    return data;
}

std::vector<ItemRenderData> ItemDataExtractor::ExtractRenderData(const std::vector<Item>& items) {
    std::vector<ItemRenderData> renderData;
    renderData.reserve(items.size());
    
    for (const auto& item : items) {
        if (item.alive) {
            renderData.push_back(ExtractRenderData(item));
        }
    }
    
    return renderData;
}

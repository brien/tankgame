#include "PlatformGL.h"

#include "ItemRenderer.h"
#include "../App.h"
#include "RenderContext.h"

ItemRenderer::ItemRenderer() {
    // Constructor - base class handles initialization
}

void ItemRenderer::SetupRenderState() {
    BaseRenderer::SetupRenderState();
    
    // Set up OpenGL state for item rendering
    glFrontFace(GL_CCW);
}

void ItemRenderer::CleanupRenderState() {
    // Restore OpenGL state
    glFrontFace(GL_CW);
    
    BaseRenderer::CleanupRenderState();
}

void ItemRenderer::RenderItems(const std::vector<ItemRenderData>& items) {
    if (items.empty()) {
        return;
    }
    
    SetupRenderState();
    
    for (const auto& item : items) {
        if (item.visible) {
            RenderItem(item);
        }
    }
    
    CleanupRenderState();
}

void ItemRenderer::RenderItem(const ItemRenderData& item) {
    if (!item.visible) {
        return;
    }
    
    const Matrix4 model = Matrix4::Translation(item.position.x, item.position.y, item.position.z) *
        Matrix4::Rotation(-item.rotationY, 0, 1, 0) * Matrix4::Rotation(90, 0, 0, 1);
    const Color color = GetItemColor(item.itemType);
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::Item), model,
                                  color.r, color.g, color.b);
}

Color ItemRenderer::GetItemColor(TankType itemType) {
    switch (itemType) {
        case TankType::TYPE_RED: return Color(1.0f, 0.0f, 0.0f);
        case TankType::TYPE_BLUE: return Color(0.0f, 0.0f, 1.0f);
        case TankType::TYPE_YELLOW: return Color(1.0f, 1.0f, 0.0f);
        case TankType::TYPE_PURPLE: return Color(1.0f, 0.0f, 1.0f);
        default: return Color(0.5f, 0.5f, 0.5f);
    }
}

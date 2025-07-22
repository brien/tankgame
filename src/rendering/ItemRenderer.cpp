#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "ItemRenderer.h"
#include "../App.h"

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
    
    glPushMatrix();
    
    // Apply transformations
    glTranslatef(item.position.x, item.position.y, item.position.z);
    glRotatef(-item.rotationY, 0, 1, 0);  // Negative for spinning animation
    glRotatef(0, 0, 0, 1);  // rz was always 0 in original code
    glRotatef(90, 0, 0, 1); // Fixed rotation from original
    
    // Set color based on item type
    SetItemColor(item.itemType);
    
    // Render the item mesh using the display list from GraphicsTask
    App::GetSingleton().graphicsTask->itemlist.Call(0);
    
    glPopMatrix();
}

void ItemRenderer::SetItemColor(TankType itemType) {
    // Set color based on item type (same logic as Item constructor)
    switch (itemType) {
        case TankType::TYPE_GREY:
            glColor3f(0.5f, 0.5f, 0.5f);
            break;
        case TankType::TYPE_RED:
            glColor3f(1.0f, 0.0f, 0.0f);
            break;
        case TankType::TYPE_BLUE:
            glColor3f(0.0f, 0.0f, 1.0f);
            break;
        case TankType::TYPE_YELLOW:
            glColor3f(1.0f, 1.0f, 0.0f);
            break;
        case TankType::TYPE_PURPLE:
            glColor3f(1.0f, 0.0f, 1.0f);
            break;
        default:
            glColor3f(0.5f, 0.5f, 0.5f);
            break;
    }
}

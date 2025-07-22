#ifndef NEWBULLETRENDERER_H
#define NEWBULLETRENDERER_H

#include "BaseRenderer.h"
#include <vector>

// Forward declaration
struct BulletRenderData;

/**
 * BulletRenderer - Handles all bullet rendering using the new rendering pipeline.
 * 
 * This class replaces the old BulletRenderer and individual Bullet::Draw() methods,
 * providing centralized, efficient bullet rendering that separates game logic
 * from OpenGL rendering code.
 * 
 * Named "BulletRenderer" to avoid conflicts during migration.
 */
class BulletRenderer : public BaseRenderer {
public:
    BulletRenderer();
    virtual ~BulletRenderer() = default;

    // IRenderer interface implementation
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    
    // Bullet-specific rendering method
    void RenderBullets(const std::vector<BulletRenderData>& bullets);

private:
    // Main rendering functions for different bullet types
    void RenderBlueBullet(const BulletRenderData& bullet);
    void RenderStandardBullet(const BulletRenderData& bullet);
    
    // Utility functions
    void SetupBulletRendering();
    void CleanupBulletRendering();
    void RenderBulletGeometry(const BulletRenderData& bullet, float yOffset, float zOffset, float rotationX, float scaleZ);
    void SetupBlendMode();
    void RestoreBlendMode();
    
    // Cached state to avoid redundant OpenGL calls
    bool blendEnabled;
    bool texturesEnabled;
};

#endif // NEWBULLETRENDERER_H

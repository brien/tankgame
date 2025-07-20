#ifndef EFFECTRENDERER_H
#define EFFECTRENDERER_H

#include "BaseRenderer.h"
#include <vector>

// Forward declaration
struct EffectRenderData;

/**
 * EffectRenderer - Handles all visual effect rendering using the new rendering pipeline.
 * 
 * This class replaces the old FX::Draw() methods, providing centralized, efficient 
 * effect rendering that separates game logic from OpenGL rendering code.
 * 
 * Supports all effect types: explosions, smoke, stars, death effects, etc.
 */
class EffectRenderer : public BaseRenderer {
public:
    EffectRenderer();
    virtual ~EffectRenderer() = default;

    // IRenderer interface implementation
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    
    // Effect-specific rendering method
    void RenderEffects(const std::vector<EffectRenderData>& effects);

private:
    // Main rendering functions for different effect types
    void RenderEffect(const EffectRenderData& effect);
    void SetupEffectTexture(const EffectRenderData& effect);
    void ApplyEffectScale(const EffectRenderData& effect);
    void RenderEffectGeometry(const EffectRenderData& effect);
    
    // Utility functions
    void SetupEffectRendering();
    void CleanupEffectRendering();
    void SetupBlendMode();
    void RestoreBlendMode();
    
    // Cached state to avoid redundant OpenGL calls
    bool blendEnabled;
    bool texturesEnabled;
    unsigned int currentTexture;
};

#endif // EFFECTRENDERER_H

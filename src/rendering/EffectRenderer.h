#ifndef EFFECTRENDERER_H
#define EFFECTRENDERER_H

#include "BaseRenderer.h"
#include <vector>
#include "ResourceManager.h"
#include "Matrix4.h"

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
    explicit EffectRenderer(ResourceManager* resources = nullptr);
    static Matrix4 BuildModel(const EffectRenderData& effect);
    static TextureNames TextureFor(FxType type);
    static bool HasOutline(FxType type);
    virtual ~EffectRenderer() = default;

    // IRenderer interface implementation
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    
    // Effect-specific rendering method
    void RenderEffects(const std::vector<EffectRenderData>& effects);

private:
    ResourceManager* resources; // Non-owning catalogue, shared with the pipeline.
    void RenderModern(const std::vector<EffectRenderData>& effects);
#ifndef __EMSCRIPTEN__
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
    
#endif
    // Cached state to avoid redundant OpenGL calls
    bool blendEnabled;
    bool texturesEnabled;
    unsigned int currentTexture;
};

#endif // EFFECTRENDERER_H

#include "PlayerTankRendererImpl.h"
#include "PlayerTankRenderer.h" // For delegating to existing static methods

PlayerTankRendererImpl::PlayerTankRendererImpl() 
    : animationDrift(0.0f) {
}

bool PlayerTankRendererImpl::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }
    animationDrift = 0.0f;
    // TODO: Initialize player-specific resources
    // - Load player effect textures
    // - Initialize targeting UI resources
    // - Set up any player-specific shaders
    return true;
}

void PlayerTankRendererImpl::Cleanup() {
    // TODO: Clean up player-specific resources
    BaseRenderer::Cleanup();
}

void PlayerTankRendererImpl::Render(const TankRenderData& data) {
    RenderOpaque(data);
    RenderOverlays(data);
}

void PlayerTankRendererImpl::RenderOpaque(const TankRenderData& data) {
    if (!data.alive || !data.isPlayer) return;
    PlayerTankRenderer::RenderPlayerTank(data, animationDrift);
}

void PlayerTankRendererImpl::RenderOverlays(const TankRenderData& data) {
    if (!data.alive || !data.isPlayer) {
        return;
    }
    
    // Update animation drift
    animationDrift += 0.016f; // Approximate 60 FPS delta time
    if (animationDrift > DRIFT_RESET_THRESHOLD * 1000.0f) {
        animationDrift = 0.0f;
    }
    
    // Render additional player-specific elements
    RenderPlayerEffects(data);
    RenderTargetingUI(data, true); // TODO: Get actual enemy presence from game state
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
}

void PlayerTankRendererImpl::SetupRenderState() {
    BaseRenderer::SetupRenderState();
    SetupPlayerTankRenderState();
}

void PlayerTankRendererImpl::CleanupRenderState() {
    BaseRenderer::CleanupRenderState();
}

void PlayerTankRendererImpl::RenderPlayerEffects(const TankRenderData& tank) {
    // Delegate to existing PlayerTankRenderer for consistency
    PlayerTankRenderer::RenderPlayerEffects(tank, animationDrift);
}

void PlayerTankRendererImpl::RenderTargetingUI(const TankRenderData& tank, bool hasEnemyTargets) {
    // Delegate to existing PlayerTankRenderer for consistency
    PlayerTankRenderer::RenderTargetingUI(tank, animationDrift, hasEnemyTargets);
}

void PlayerTankRendererImpl::SetupPlayerTankRenderState() {
    // Standard player tank rendering state
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glFrontFace(GL_CW);
}

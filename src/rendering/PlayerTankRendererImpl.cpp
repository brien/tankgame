#include "PlayerTankRendererImpl.h"
#include "PlayerTankRenderer.h" // For delegating to existing static methods
#include "../App.h"

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
    if (!data.alive || !data.isPlayer) {
        return;
    }
    
    // Update animation drift
    animationDrift += 0.016f; // Approximate 60 FPS delta time
    if (animationDrift > DRIFT_RESET_THRESHOLD * 1000.0f) {
        animationDrift = 0.0f;
    }
    
    // Delegate to existing PlayerTankRenderer static methods for now
    // TODO: Implement native rendering when PlayerTankRenderer is fully refactored
    PlayerTankRenderer::RenderPlayerTank(data, animationDrift);
    
    // Render additional player-specific elements
    RenderPlayerEffects(data);
    RenderTargetingUI(data, true); // TODO: Get actual enemy presence from game state
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
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glFrontFace(GL_CW);
}

void PlayerTankRendererImpl::SetupPlayerEffectsRenderState() {
    // Effects rendering state (from PlayerTankRenderer)
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void PlayerTankRendererImpl::SetupTargetingUIRenderState() {
    // Targeting UI rendering state (from PlayerTankRenderer)
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
}

// TODO: Implement individual rendering methods when fully separating from static PlayerTankRenderer
void PlayerTankRendererImpl::RenderTankBody(const TankRenderData& tank) {
    // To be implemented when moving away from static PlayerTankRenderer
}

void PlayerTankRendererImpl::RenderTankTurret(const TankRenderData& tank) {
    // To be implemented when moving away from static PlayerTankRenderer
}

void PlayerTankRendererImpl::RenderEffectBody(const TankRenderData& tank) {
    // To be implemented when moving away from static PlayerTankRenderer
}

void PlayerTankRendererImpl::RenderEffectTurret(const TankRenderData& tank) {
    // To be implemented when moving away from static PlayerTankRenderer
}

void PlayerTankRendererImpl::RenderTargetingIndicator(const TankRenderData& tank) {
    // To be implemented when moving away from static PlayerTankRenderer
}

void PlayerTankRendererImpl::RenderReadyIndicator(const TankRenderData& tank) {
    // To be implemented when moving away from static PlayerTankRenderer
}

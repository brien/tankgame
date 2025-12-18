#include "EffectDataExtractor.h"

std::vector<EffectRenderData> EffectDataExtractor::ExtractEffectRenderData(const std::vector<FX>& effects) {
    std::vector<EffectRenderData> renderData;
    renderData.reserve(effects.size());
    
    for (const FX& effect : effects) {
        if (effect.alive) {
            renderData.push_back(ExtractSingleEffectData(effect));
        }
    }
    
    return renderData;
}

EffectRenderData EffectDataExtractor::ExtractSingleEffectData(const FX& effect) {
    EffectRenderData data;
    
    // Basic properties
    data.type = effect.type;
    
    // Position
    data.position.x = effect.x;
    data.position.y = effect.y;
    data.position.z = effect.z;
    
    // Rotation
    data.rotation.x = effect.rx;
    data.rotation.y = effect.ry;
    data.rotation.z = effect.rz;
    
    // Velocity (movement direction)
    data.velocity.x = effect.dx;
    data.velocity.y = effect.dy;
    data.velocity.z = effect.dz;
    
    // Visual properties
    data.r = effect.color.r;
    data.g = effect.color.g;
    data.b = effect.color.b;
    data.alpha = effect.color.a;
    
    // Scale calculation based on time progression (from original FX::Draw logic)
    float timeRatio = (effect.maxTime > 0) ? (effect.time / effect.maxTime) : 1.0f;
    
    switch (effect.type) {
        case FxType::TYPE_ZERO:
            data.scale = 5.0f * timeRatio;
            break;
        case FxType::TYPE_JUMP:
            data.scale = 1.0f * timeRatio;
            break;
        case FxType::TYPE_SMOKE:
            data.scale = 0.25f * timeRatio;
            break;
        case FxType::TYPE_SMALL_SQUARE:
            data.scale = 0.5f * timeRatio;
            break;
        case FxType::TYPE_STAR:
            data.scale = 0.3f * timeRatio;
            break;
        case FxType::TYPE_SMALL_RECTANGLE:
            data.scale = 1.0f; // Special scaling handled in renderer
            break;
        default:
            data.scale = 1.0f;
            break;
    }
    
    // Lifetime for fade calculations
    data.lifetime = effect.maxTime - effect.time;
    
    return data;
}

#include "EffectRenderer.h"
#include "RenderData.h"
#include "PlatformGL.h"
#include "RenderContext.h"
#include "../App.h"
#include "../Logger.h"

EffectRenderer::EffectRenderer(ResourceManager* catalogue) :
    BaseRenderer(),
    resources(catalogue)
{
}

bool EffectRenderer::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }
    
    
    Logger::Get().Write("EffectRenderer initialized successfully\n");
    return true;
}

void EffectRenderer::Cleanup() {
    BaseRenderer::Cleanup();
    Logger::Get().Write("EffectRenderer cleaned up\n");
}

void EffectRenderer::RenderEffects(const std::vector<EffectRenderData>& effects) {
    if (!isInitialized) {
        Logger::Get().Write("ERROR: EffectRenderer not initialized\n");
        return;
    }
    
    if (effects.empty()) {
        return;
    }
    
    RenderModern(effects);
}

Matrix4 EffectRenderer::BuildModel(const EffectRenderData& effect) {
    float x = 1, y = 1, z = 1;
    switch (effect.type) {
        case FxType::TYPE_ZERO:
        case FxType::TYPE_JUMP: x = z = effect.scale; break;
        case FxType::TYPE_SMOKE: x = z = effect.scale; y = .25f; break;
        case FxType::TYPE_SMALL_SQUARE: x = z = effect.scale; y = .5f; break;
        case FxType::TYPE_STAR: x = z = effect.scale; y = .3f; break;
        case FxType::TYPE_SMALL_RECTANGLE: x = .02f; z = .2f; break;
        default: break;
    }
    return Matrix4::Translation(effect.position.x, effect.position.y + .2f, effect.position.z) *
        Matrix4::Rotation(effect.rotation.x, 1, 0, 0) *
        Matrix4::Rotation(-effect.rotation.y, 0, 1, 0) *
        Matrix4::Rotation(effect.rotation.z, 0, 0, 1) * Matrix4::Scale(x, y, z);
}

TextureNames EffectRenderer::TextureFor(FxType type) {
    if (type == FxType::TYPE_THREE) return TEXTURE_HEART;
    if (type == FxType::TYPE_STAR) return TEXTURE_STAR;
    return TEXTURE_NAMES_COUNT;
}

bool EffectRenderer::HasOutline(FxType type) {
    return type == FxType::TYPE_ZERO || type == FxType::TYPE_DEATH;
}

void EffectRenderer::RenderModern(const std::vector<EffectRenderData>& effects) {
    ResourceManager* catalogue = resources;
    if (!catalogue && App::GetSingleton().graphicsTask)
        catalogue = &App::GetSingleton().graphicsTask->Resources();
    if (!catalogue) return;

    // Preserve compatibility submission order: opaque outlines followed by
    // additive quads. Quads test opaque depth but never write it.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    const auto restore = [] {
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
    };
    try {
        for (const auto& effect : effects) {
            const Matrix4 model = BuildModel(effect);
            BasicMaterial material;
            material.red = effect.r; material.green = effect.g; material.blue = effect.b;
            const auto texture = TextureFor(effect.type);
            if (texture != TEXTURE_NAMES_COUNT)
                material.texture = catalogue->GetTextureHandler().GetTexture(texture);
            if (HasOutline(effect.type)) {
                glDisable(GL_BLEND);
                glDepthMask(GL_TRUE);
                RenderContext::Current().Draw(catalogue->GetGeometry(GeometryResource::HorizontalOutline), model, material);
            }
            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);
            material.alpha = effect.alpha;
            RenderContext::Current().Draw(catalogue->GetGeometry(GeometryResource::HorizontalQuad), model, material);
        }
    } catch (...) {
        restore();
        throw;
    }
    restore();
}

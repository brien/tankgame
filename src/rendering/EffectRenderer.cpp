#include "EffectRenderer.h"
#include "RenderData.h"
#include "PlatformGL.h"
#include "RendererMode.h"
#include "RenderContext.h"
#include "../App.h"
#include "../Logger.h"

EffectRenderer::EffectRenderer(ResourceManager* catalogue) :
    BaseRenderer(),
    resources(catalogue),
    blendEnabled(false),
    texturesEnabled(false),
    currentTexture(0)
{
}

bool EffectRenderer::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }
    
    blendEnabled = false;
    texturesEnabled = false;
    currentTexture = 0;
    
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
    
    if (RendererMode::IsModern()) {
        RenderModern(effects);
        return;
    }
#ifndef __EMSCRIPTEN__
    // Save OpenGL state
    SetupRenderState();
    
    try {
        SetupEffectRendering();
        
        // Render all effects
        for (const auto& effect : effects) {
            RenderEffect(effect);
        }
        
        CleanupEffectRendering();
        
    } catch (...) {
        Logger::Get().Write("ERROR: Exception occurred during effect rendering\n");
    }
    
    // Restore OpenGL state
    CleanupRenderState();
#endif
}

#ifndef __EMSCRIPTEN__
void EffectRenderer::RenderEffect(const EffectRenderData& effect) {
    glPushMatrix();
    
    // Position the effect
    glTranslatef(effect.position.x, effect.position.y + 0.2f, effect.position.z);
    
    // Apply rotations
    glRotatef(effect.rotation.x, 1, 0, 0);
    glRotatef(-effect.rotation.y, 0, 1, 0);
    glRotatef(effect.rotation.z, 0, 0, 1);
    
    // Set up textures for specific effect types
    SetupEffectTexture(effect);
    
    // Disable face culling for effects
    glDisable(GL_CULL_FACE);
    
    // Apply effect-specific scaling
    ApplyEffectScale(effect);
    
    // Set primary color for outline/base
    glColor3f(effect.r, effect.g, effect.b);
    
    // Render base geometry for certain types
    RenderEffectGeometry(effect);
    
    // Render glowing effect with alpha blending
    SetupBlendMode();
    glColor4f(effect.r, effect.g, effect.b, effect.alpha);
    
    // Render the glowing inner part
    if (App::GetSingleton().graphicsTask) {
        App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::HorizontalQuad).Call(0);
    }
    
    RestoreBlendMode();
    
    // Clean up textures and face culling
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    
    glPopMatrix();
}

void EffectRenderer::SetupEffectTexture(const EffectRenderData& effect) {
    unsigned int textureId = 0;
    
    switch (effect.type) {
        case FxType::TYPE_THREE:
            textureId = 16;
            break;
        case FxType::TYPE_STAR:
            textureId = 19;
            break;
        default:
            // No texture needed for other effect types
            return;
    }
    
    if (textureId != 0 && App::GetSingleton().graphicsTask) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->Resources().GetTextureHandler().GetTextureArray()[textureId]);
        texturesEnabled = true;
        currentTexture = textureId;
    }
}

void EffectRenderer::ApplyEffectScale(const EffectRenderData& effect) {
    switch (effect.type) {
        case FxType::TYPE_ZERO:
            glScalef(effect.scale, 1, effect.scale);
            break;
        case FxType::TYPE_JUMP:
            glScalef(effect.scale, 1, effect.scale);
            break;
        case FxType::TYPE_SMOKE:
            glScalef(effect.scale, 0.25f, effect.scale);
            break;
        case FxType::TYPE_SMALL_SQUARE:
            glScalef(effect.scale, 0.5f, effect.scale);
            break;
        case FxType::TYPE_STAR:
            glScalef(effect.scale, 0.3f, effect.scale);
            break;
        case FxType::TYPE_SMALL_RECTANGLE:
            glScalef(0.02f, 1, 0.2f);
            break;
        default:
            // No scaling for other types
            break;
    }
}

void EffectRenderer::RenderEffectGeometry(const EffectRenderData& effect) {
    // Render base geometry for specific effect types
    if (effect.type == FxType::TYPE_DEATH || effect.type == FxType::TYPE_ZERO) {
        if (App::GetSingleton().graphicsTask) {
            App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::HorizontalOutline).Call(0);
        }
    }
}

void EffectRenderer::SetupEffectRendering() {
    // Emissive effects must not inherit lighting from the preceding object.
    glDisable(GL_LIGHTING);
    // Effects generally don't need specific global setup
    blendEnabled = false;
    texturesEnabled = false;
    currentTexture = 0;
}

void EffectRenderer::CleanupEffectRendering() {
    // Restore any global state if needed
    if (texturesEnabled) {
        glDisable(GL_TEXTURE_2D);
        texturesEnabled = false;
    }
}

void EffectRenderer::SetupBlendMode() {
    if (!blendEnabled) {
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        blendEnabled = true;
    }
}

void EffectRenderer::RestoreBlendMode() {
    if (blendEnabled) {
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        blendEnabled = false;
    }
}

#endif

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
